#ifndef DUT_H_
#define DUT_H_

struct dut_ops {
		void (*init)(int queue_len, int worker_cnt);
		int (*enqueue)(void *data, int queue_id);
		int (*dequeue)(void **data, int queue_id);
};

#endif /* DUT_H_ */
