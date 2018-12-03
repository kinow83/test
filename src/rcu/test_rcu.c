#include <stdio.h>
#include <urcu.h>		/* Userspace RCU flavor */
#include <urcu/rculist.h>	/* RCU list */
#include <urcu/compiler.h>	/* For CAA_ARRAY_SIZE */
#include <pthread.h>
#include <unistd.h>


struct mynode {
	int value;			/* Node content */
	struct cds_list_head node;	/* Linked-list chaining */
	struct rcu_head rcu_head;	/* For call_rcu() */
};

static CDS_LIST_HEAD(mylist);


void* reader(void *arg)
{
	printf("reader\n");

	rcu_register_thread();

	struct cds_list_head *pos;

	rcu_read_lock();

	cds_list_for_each_rcu(pos, &mylist)
	{
		struct mynode *node = cds_list_entry(pos, struct mynode, node);
		printf("read: %d\n", node->value);
		sleep(1);
	}

	rcu_read_unlock();

	while (1) {
		sleep(1);
	}
	return NULL;
}


static void free_node_rcu(struct rcu_head *head)
{
	printf("free_node_rcu\n");
	struct mynode *node = caa_container_of(head, struct mynode, rcu_head);
	free(node);
}

void* writer(void *arg)
{
	rcu_register_thread();
	printf("writer\n");

	struct mynode *node, *n;
	cds_list_for_each_entry_safe(node, n, &mylist, node)
	{
		if (node->value > 0) {
			cds_list_del_rcu(&node->node);
			printf("delete: %d\n", node->value);
			call_rcu(&node->rcu_head, free_node_rcu);

			sleep(1);
		}
	}

	while (1) {
		sleep(1);
	}
	return NULL;
}



int main()
{
	int i = 0;
	struct cds_list_head *pos;

	rcu_register_thread();

	for (i=0; i<10; i++) {
		struct mynode *node = malloc(sizeof(*node));
		node->value = i;
		cds_list_add_tail_rcu(&node->node, &mylist);
		printf("add %d\n", node->value);
	}

	rcu_read_lock();
	cds_list_for_each_rcu(pos, &mylist)
	{
		struct mynode *node = cds_list_entry(pos, struct mynode, node);
		printf(" %d\n", node->value);
	}
	rcu_read_unlock();

	pthread_t t_r, t_w;
	pthread_create(&t_r, NULL, reader, NULL);
	pthread_create(&t_w, NULL, writer, NULL);

	pthread_join(t_r, NULL);
	pthread_join(t_w, NULL);

	return 0;
}
