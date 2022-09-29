/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"
#include <string.h>
#define NR_WP 32

//将这里的WP定义放到了sdb.h

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp( char* expr ) {
	if (free_ == NULL) {
		fprintf(stderr,"lack of free watch point.\n");
		panic();
		Assert(0,"\n");
	}
	WP* tem = free_;
	free_ = free_->next;
	tem->next = head;
	head = tem;
	strcpy(tem->expr, expr);
	return head;
}

void free_wp(WP *wp) {
	int i;
	if (head == wp) {
		head = wp->next;
	}
	else {
		for (i=0; head[i].next != wp && head[i].next != NULL; i++) {}
		if (head[i].next == NULL) { panic(); }
		head[i].next = wp->next;
	}
	wp->next = NULL;
	if (free_ == NULL)	{
		free_ = wp;
	}
	else {
		wp->next = free_;
		free_ = wp;
	}
}
