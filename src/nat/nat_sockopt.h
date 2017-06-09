/*
 * nat_sockopt.h
 *
 *  Created on: 2017. 6. 9.
 *      Author: root
 */

#ifndef SRC_NAT_NAT_SOCKOPT_H_
#define SRC_NAT_NAT_SOCKOPT_H_


#define NAT_BASE_CTL 128

#define NAT_SO_SET_CONFIG  (NAT_BASE_CTL)
#define NAT_SO_SET_TEST    (NAT_BASE_CTL + 1)
#define NAT_SO_SET_NAME    (NAT_BASE_CTL + 2)
#define NAT_SO_SET_ID      (NAT_BASE_CTL + 3)
#define NAT_SO_SET_ALL     (NAT_BASE_CTL + 4)
#define NAT_SO_SET_MAX     (NAT_BASE_CTL + 5)

#define NAT_SO_GET_CONFIG  (NAT_BASE_CTL)
#define NAT_SO_GET_TEST    (NAT_BASE_CTL + 1)
#define NAT_SO_GET_NAME    (NAT_BASE_CTL + 2)
#define NAT_SO_GET_ID      (NAT_BASE_CTL + 3)
#define NAT_SO_GET_ALL     (NAT_BASE_CTL + 4)
#define NAT_SO_GET_MAX     (NAT_BASE_CTL + 5)


struct nat_so_opt_t {
	int id;
	char name[128];
};



#endif /* SRC_NAT_NAT_SOCKOPT_H_ */
