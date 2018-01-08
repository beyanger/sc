
#include <linux/module.h>  
#include <linux/init.h>  
#include <linux/moduleparam.h>  
  
#include <linux/sched.h>  
#include <linux/kernel.h> /* PDEBUG() */  
#include <linux/slab.h> /* kmalloc() */  
#include <linux/errno.h>  /* error codes */  
#include <linux/types.h>  /* size_t */  
#include <linux/interrupt.h> /* mark_bh */  
  
#include <linux/in.h>  
#include <linux/netdevice.h>   /* struct device, and other headers */  
#include <linux/etherdevice.h> /* eth_type_trans */  
#include <linux/ip.h>          /* struct iphdr */  
#include <linux/tcp.h>         /* struct tcphdr */  
#include <linux/skbuff.h>  


#include <linux/in6.h>
#include <asm/checksum.h>


#define SNULL_RX_INTR 0x0001 
#define SNULL_TX_INTR 0x0002
#define SNULL_TIMEOUT (3*HZ)


#undef PDEBUG             /* undef it, just in case */  
#define SNULL_DEBUG  
#ifdef SNULL_DEBUG  
     /* This one if debugging is on, and kernel space */  
#  define PDEBUG(fmt, args...) printk( KERN_DEBUG "snull: " fmt, ## args)  
#else  
#  define PDEBUG(fmt, args...) /* not debugging: nothing */  
#endif  
  
#undef PDEBUGG  
#define PDEBUGG(fmt, args...) printk( KERN_DEBUG "snull: " fmt, ## args)/* nothing: it's a placeholder */  
  

static int lockup = 0;
static int timeout = SNULL_TIMEOUT;
static int __attribute__((unused)) use_napi = 0;
static int pool_size = 8;

struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	int datalen;
	u8 data[ETH_DATA_LEN];
};


struct snull_priv {
	struct net_device_stats stats;
	int status;
	struct snull_packet *ppool;
	struct snull_packet *rx_queue;
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	struct napi_struct napi;
	spinlock_t lock;
};



static void snull_tx_timeout(struct net_device *dev);
static void (*snull_interrupt)(int, void *, struct pt_regs *);


void snull_setup_pool(struct net_device *dev) {
	struct snull_priv *priv = netdev_priv(dev);
	int i;
	struct snull_packet *pkt;
	priv->ppool = NULL;
	for (i = 0; i < pool_size; i++) {
		pkt = kmalloc(sizeof(struct snull_packet), GFP_KERNEL);
		if (pkt == NULL) {
			 PDEBUG (KERN_NOTICE "Ran out of memory allocating packet pool\n"); 
			 return;
		}
		pkt->dev = dev;
		pkt->next = priv->ppool;
		priv->ppool = pkt;
	}
}


void snull_teardown_pool(struct net_device *dev) {
	struct snull_priv *priv = netdev_priv(dev);
	struct snull_packet *pkt;

	while ((pkt = priv->ppool)) {
		priv->ppool = pkt->next;
		kfree(pkt);
	}
}

struct snull_packet *snull_get_tx_buffer(struct net_device *dev) {
	struct snull_priv *priv = netdev_priv(dev);
	unsigned long flags;
	struct snull_packet *pkt;

	spin_lock_irqsave(&priv->lock, flags);

	pkt = priv->ppool;
	priv->ppool = pkt->next;
	if (priv->ppool == NULL) {
		PDEBUG (KERN_INFO "Pool empty\n");  
		netif_stop_queue(dev);  
	}
	spin_unlock_irqrestore(&priv->lock, flags);  
	return pkt;  
}

void snull_release_buffer(struct snull_packet *pkt)  
{  
	unsigned long flags;  
	struct snull_priv *priv = netdev_priv(pkt->dev);  

	spin_lock_irqsave(&priv->lock, flags);  
	pkt->next = priv->ppool;  
	priv->ppool = pkt;  
	spin_unlock_irqrestore(&priv->lock, flags);  
	if (netif_queue_stopped(pkt->dev) && pkt->next == NULL)  
		netif_wake_queue(pkt->dev);  
}  

void snull_enqueue_buf(struct net_device *dev, struct snull_packet *pkt)  
{  
	unsigned long flags;  
	struct snull_priv *priv = netdev_priv(dev);  

	spin_lock_irqsave(&priv->lock, flags);  
	pkt->next = priv->rx_queue;  /* FIXME - misorders packets */  
	priv->rx_queue = pkt;  
	spin_unlock_irqrestore(&priv->lock, flags);  
}  

struct snull_packet *snull_dequeue_buf(struct net_device *dev)  
{  
	struct snull_priv *priv = netdev_priv(dev);  
	struct snull_packet *pkt;  
	unsigned long flags;  

	spin_lock_irqsave(&priv->lock, flags);  
	pkt = priv->rx_queue;  
	if (pkt != NULL)  
		priv->rx_queue = pkt->next;  
	spin_unlock_irqrestore(&priv->lock, flags);  
	return pkt;  
}  





static void snull_rx_ints(struct net_device *dev, int enable)  
{  
	struct snull_priv *priv = netdev_priv(dev);  
	priv->rx_int_enabled = enable;  
}  

struct net_device *snull_devs[2];



int snull_open(struct net_device *dev) {

	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	if (dev == snull_devs[1]) 
		dev->dev_addr[ETH_ALEN-1]++;
	netif_start_queue(dev);

	return 0;
}



int snull_release(struct net_device *dev) {
	netif_stop_queue(dev);
	return 0;
}

int snull_config(struct net_device *dev, struct ifmap *map) {
	if (dev->flags & IFF_UP) /* can't act on a running interface */  
		return -EBUSY;  

	/* Don't allow changing the I/O address */  
	if (map->base_addr != dev->base_addr) {  
		PDEBUG(KERN_WARNING "snull: Can't change I/O address\n");  
		return -EOPNOTSUPP;  
	}  

	/* Allow changing the IRQ */  
	if (map->irq != dev->irq) {  
		dev->irq = map->irq;  
		/* request_irq() is delayed to open-time */  
	}  

	/* ignore other fields */  
	return 0;  
}




void snull_rx(struct net_device *dev, struct snull_packet *pkt) {
	struct sk_buff *skb;
	struct snull_priv *priv = netdev_priv(dev);


	skb = dev_alloc_skb(pkt->datalen + 2);
	if (!skb) {
		if (printk_ratelimit())  
			PDEBUG(KERN_NOTICE "snull rx: low on mem - packet dropped\n");  
		priv->stats.rx_dropped++;  
		return;
	}

	skb_reserve(skb, 2);
	memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	priv->stats.rx_packets++;
	priv->stats.rx_bytes += pkt->datalen;
	netif_rx(skb);
}





static void snull_regular_interrupt(int irq, void *dev_id, struct pt_regs *regs)  
{  
	int statusword;  
	struct snull_priv *priv;  
	struct snull_packet *pkt = NULL;  
	/*  
	 *       * As usual, check the "device" pointer to be sure it is  
	 *            * really interrupting.  
	 *                 * Then assign "struct device *dev"  
	 *                      */  
	struct net_device *dev = (struct net_device *)dev_id;  
	/* ... and check with hw if it's really ours */  

	/* paranoid */  
	if (!dev)  
		return;  

	/* Lock the device */  
	priv = netdev_priv(dev);  
	spin_lock(&priv->lock);  

	/* retrieve statusword: real netdevices use I/O instructions */  
	statusword = priv->status;  
	priv->status = 0;  
	if (statusword & SNULL_RX_INTR) {  
		/* send it to snull_rx for handling */  
		pkt = priv->rx_queue;  
		if (pkt) {  
			priv->rx_queue = pkt->next;  
			snull_rx(dev, pkt);  
		}  
	}  
	if (statusword & SNULL_TX_INTR) {  
		/* a transmission is over: free the skb */  
		priv->stats.tx_packets++;  
		priv->stats.tx_bytes += priv->tx_packetlen;  
		dev_kfree_skb(priv->skb);  
	}  

	/* Unlock the device and we are done */  
	spin_unlock(&priv->lock);  
	if (pkt) snull_release_buffer(pkt); /* Do this outside the lock! */  
	return;  
}  



static void snull_hw_tx(char *buf, int len, struct net_device *dev) {
	struct iphdr *ih;
	struct net_device *dest;
	struct snull_priv *priv;
	u32 *saddr, *daddr;
	struct snull_packet *tx_buffer;

	if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) {
		PDEBUG("snull: Hmm... packet too short (%i octets)\n",  
				len);  
		return;  
	}


	ih = (struct iphdr *)(buf+sizeof(struct ethhdr));  
	saddr = &ih->saddr;  
	daddr = &ih->daddr; 


	((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */  
	((u8 *)daddr)[2] ^= 1;  

	ih->check = 0;         /* and rebuild the checksum (ip needs it) */  
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);  
	dest = snull_devs[dev == snull_devs[0] ? 1 : 0];  
	priv = netdev_priv(dest);  
	tx_buffer = snull_get_tx_buffer(dev);  
	tx_buffer->datalen = len;  
	memcpy(tx_buffer->data, buf, len);  
	snull_enqueue_buf(dest, tx_buffer);  
	if (priv->rx_int_enabled) {  
		priv->status |= SNULL_RX_INTR;  
		snull_interrupt(0, dest, NULL);  
	}  


	priv = netdev_priv(dev);  
	priv->tx_packetlen = len;  
	priv->tx_packetdata = buf;  
	priv->status |= SNULL_TX_INTR;  

	if (lockup && ((priv->stats.tx_packets + 1) % lockup) == 0) {  
		/* Simulate a dropped transmit interrupt */  
		netif_stop_queue(dev);  

		PDEBUG("Simulate lockup at %ld, txp %ld\n", jiffies,  
				(unsigned long) priv->stats.tx_packets);  
	}  
	else  
	{  
		snull_interrupt(0, dev, NULL);  
	}  
}

int snull_tx(struct sk_buff *skb, struct net_device *dev) {
	int len;
	char *data, shortpkt[ETH_ZLEN];

	struct snull_priv *priv = netdev_priv(dev);

	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, sizeof(shortpkt));
		memcpy(shortpkt, skb->data, skb->len);
		len = ETH_ZLEN;
		data = shortpkt;
	}
	dev->trans_start = jiffies;
	priv->skb = skb;
	snull_hw_tx(data, len, dev);
	return 0;
}


void snull_tx_timeout(struct net_device *dev) {
	struct snull_priv *priv = netdev_priv(dev);

	priv->status = SNULL_TX_INTR;
	snull_interrupt(0, dev, NULL);
	priv->stats.tx_errors++;
	netif_wake_queue(dev);
}


int snull_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {
	return 0;
}


struct net_device_stats *snull_stats(struct net_device *dev) {
	struct snull_priv *priv = netdev_priv(dev);
	return &priv->stats;
}



int snull_rebuild_header(struct sk_buff *skb) {
	struct ethhdr *eth = (struct ethhdr *)skb->data;
	struct net_device *dev = skb->dev;

	memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest, dev->dev_addr, dev->addr_len);
	eth->h_dest[ETH_ALEN-1] ^= 1;
	return 0;
}


int snull_header(struct sk_buff *skb, struct net_device *dev, 
		unsigned short type, const void *daddr,
		const void *saddr, unsigned len) {
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr?saddr:dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest, daddr?daddr:dev->dev_addr, dev->addr_len);
	eth->h_dest[ETH_ALEN-1] ^= 1;
	return dev->hard_header_len;
}


int snull_change_mtu(struct net_device *dev, int new_mtu) {
	unsigned long flags;  
	struct snull_priv *priv = netdev_priv(dev);  
	spinlock_t *lock = &priv->lock;  

	/* check ranges */  
	if ((new_mtu < 68) || (new_mtu > 1500))  
		return -EINVAL;  
	/*  
	 *       * Do anything you need, and the accept the value  
	 *            */  
	spin_lock_irqsave(lock, flags);  
	dev->mtu = new_mtu;  
	spin_unlock_irqrestore(lock, flags);  
	return 0; /* success */  
}

static struct header_ops header_devops =  
{  
	.create = snull_header,  
};  


static struct net_device_ops net_devops =  
{  
	.ndo_open            = snull_open,  
	.ndo_stop            = snull_release,  
	.ndo_set_config      = snull_config,  
	.ndo_start_xmit      = snull_tx,  
	.ndo_do_ioctl        = snull_ioctl,  
	.ndo_get_stats       = snull_stats,  
	.ndo_change_mtu      = snull_change_mtu,    
	.ndo_tx_timeout      = snull_tx_timeout,  
};  


void snull_init(struct net_device *dev) {
	struct snull_priv *priv;

	ether_setup(dev);
	dev->header_ops = &header_devops;
	dev->netdev_ops = &net_devops;
	dev->watchdog_timeo = timeout;

	priv = netdev_priv(dev);
	memset(priv, 0, sizeof(*priv));


	dev->flags |= IFF_NOARP;
//	dev->features |= NETIF_F_NO_CSUM;

	spin_lock_init(&priv->lock);
	snull_rx_ints(dev, 1);
	snull_setup_pool(dev);
}

void snull_module_cleanup(void) {
	int i;
	for (i = 0; i < 2; i++) {
		if (snull_devs[i]) {
			unregister_netdev(snull_devs[i]);
			snull_teardown_pool(snull_devs[i]);
			free_netdev(snull_devs[i]);
		}
	}
}


int  snull_module_init(void) {
	int result, i, ret = -ENOMEM;
	char name[128];
	snull_interrupt = snull_regular_interrupt;
	for (i = 0; i < 2; i++) {
		snprintf(name, sizeof(name), "snull%d", i);

		snull_devs[i] = alloc_netdev(sizeof(struct snull_priv), name, (char)i, snull_init);
	}

	if (snull_devs[0] == NULL || snull_devs[1] == NULL) 
		goto out;

	ret = -ENODEV;
	for (i = 0; i < 2; i++) {
		if ((result = register_netdev(snull_devs[i])))
			PDEBUG("snull: error %i registering device \"%s\"\n",  
					result, snull_devs[i]->name);  
		else 
			ret = 0;
	}



out:
	if (ret)
		snull_module_cleanup();
	return ret;
}



module_init(snull_module_init);
module_exit(snull_module_cleanup);

MODULE_AUTHOR("Yang");
MODULE_LICENSE("GPL");

