﻿/*
 *  jvt_bucket.h 
 *  jvt_bucket
 *
 *  Created by Jevstein on 2018/8/2 18:39.
 *  Copyright @ 2018year Jevstein. All rights reserved.
 *
 *  桶排序(箱排序)思想：
 *        将数组分到有限数量的桶子里。每个桶子再个别排序（有可能再使用别的排序算法或是以
 *     递归方式继续使用桶排序进行排序）。桶排序是鸽巢排序的一种归纳结果。当要被排序的数
 *     组内的数值是均匀分配的时候，桶排序使用线性时间（Θ（n））。但桶排序并不是比较排序，
 *     他不受到 O(n log n) 下限的影响。
 *
 *     *.复杂度：t=O(n+k)，[t1=O(n+k), t2=O(n^2)]; s=O(n+k)
 *     *.稳定
 * 	   *.out-place
 *
 *     [运用场景]: 元素是从一个密集集合中抽取出
 *        利用函数的映射关系，减少了几乎所有的比较工作。以下来自百度：
 *        【运用1】：海量数据--对一年全国高考考生500万人数(使用标准分，最低100，最高900，
 *     无小数)排序。
 *        [分析]: 若基于比较的先进排序，平均比较次数为O(5000000*log5000000)≈1.112亿。但
 *     这些数据都有特殊的条件：100=<score<=900，则可以考虑桶排序这样一个“投机取巧”的办
 *     法、让其在毫秒级别就完成500万排序。
 *        [方法]: 创建801(900-100)个桶。将每个考生的分数丢进f(score)=score-100的桶中。这
 *     个过程从头到尾遍历一遍数据只需要500W次。然后根据桶号大小依次将桶中数值输出，即可以
 *     得到一个有序的序列。而且可以很容易的得到100分有*人，501分有*人。
 *        【运用2】：在一个文件中有10G(即10,737,418,240)个整数，乱序排列，要求找出中位
 *     数(内存限制为2G)。(中位数如: 奇数={1,3,3,6,7,8,9}, median = 6;
 *     偶数={1,2,3,4,5,6}, median = (3 + 4) / 2 = 3.5 )
 *        [思想]：将整型的每1byte作为一个关键字，也就是说一个整形可以拆成4个keys，而且最
 *     高位的keys越大，整数越大。如果高位keys相同，则比较次高位的keys。整个比较过程类似于
 *     字符串的字典序。
 *        [方法]:
 *        1.把10G整数每2G读入一次内存，然后一次遍历这536,870,912即(1024*1024*1024)*2 /4个数据。
 *     每个'数据'用位运算">>"取出最高8位(31-24)。这8bits(0-255)最多表示256个桶，则可以根据8bit
 *     的值来确定将这个'数据'丢入第几个桶。最后把每个桶写入一个磁盘文件中，同时在内存中统计每
 *     个桶内数据的数量NUM[256].
 *          代价：1)10G数据依次读入内存的IO代价(这个是无法避免的，CPU不能直接在磁盘上运算)。2)
 *     在内存中遍历536,870,912(0.5G)个数据，这是一个O(n)的线性时间复杂度。3)把256个桶写回到256
 *     个磁盘文件空间中，这个代价是额外的，也就是多付出一倍的10G数据转移的时间。
 *        2.根据内存中256个桶内的数量NUM[256]，计算中位数在第几个桶中。很显然，2,684,354,560
 *     (10G/4byte)个数中位数是第1,342,177,280个。假设前127个桶的数据量相加发现少于1,342,177,280，
 *     把第128个桶数据量加上则大于1,342,177,280，说明中位数必在磁盘的第128个桶中。而且在这个
 *     桶的第1,342,177,280-N(0-127)个数位上。N(0-127)表示前127个桶的数据量之和。然后把第128个
 *     文件中的整数读入内存。(若数据大致是均匀分布的，每个文件的大小估计在10G/256=40M左右，当
 *     然也不一定，但是超过2G的可能性很小)。注意，变态的情况下，这个需要读入的第128号文件仍然
 *     大于2G，那么整个读入仍然可以按照第一步分批来进行读取。
 *          代价：1)循环计算255个桶中的数据量累加，需要O(M)的代价，其中m<255。(2)读入一个大概
 *     80M左右文件大小的IO代价。
 *        3.继续以内存中的某个桶内整数的次高8bit（他们的最高8bit是一样的）进行桶排序(23-16)。
 *     过程和第一步相同，也是256个桶。
 *        4.一直下去，直到最低字节(7-0bit)的桶排序结束。我相信这个时候完全可以在内存中使用一次
 *     快排就可以了。
 *        整个过程的时间复杂度在O(n)的线性级别上(没有任何循环嵌套)。但主要时间消耗在第一步的第
 *     二次内存-磁盘数据交换上，即10G数据分255个文件写回磁盘上。一般而言，如果第二步过后，内存
 *     可以容纳下存在中位数的某一个文件的话，直接快排就可以了（修改者注：我想，继续桶排序但不写
 *     回磁盘，效率会更高？）
 *
 *     [缺陷]: 需要新建一数组或链表，作为辅助空间
 */

#ifndef _JVT_BUCKET_H_
#define _JVT_BUCKET_H_
#include "../../jvt_algorithm.h"

// 桶排序
void jvt_bucket_sort(jvt_datas_t *datas);


typedef struct _bkt_node
{
	JVT_KEY_TYPE value;
	struct _bkt_node * next;
} bkt_node_t;

void jvt_bucket_sort(jvt_datas_t *datas)
{
	/*const*/ int n = 10;//桶个数
	bkt_node_t *node = NULL, *t = NULL;

	//1.创建桶
	bkt_node_t *bucket = (bkt_node_t *)calloc(n, sizeof(bkt_node_t));
	assert(bucket);
	for (int i = 0; i < n; i++)
	{
		bucket[i].next = NULL;
	}

	//2.插入桶
	for (int i = 0; i < datas->size; i++)
	{
		node = (bkt_node_t *)malloc(sizeof(bkt_node_t));
		node->value = datas->data[i];
		node->next = NULL;

		t = &bucket[datas->data[i] / n];
		while (t != NULL)
		{
			if (t->next == NULL)
			{//1) 桶数据为空 => 后插
				t->next = node;
				break;
			}

			if (t->next->value > node->value)
			{//2) '桶数据' > '当前数据' => 前插
				node->next = t->next;
				t->next = node;
				break;
			}

			t = t->next;
		}
	}

	//3.将桶数据拷贝到‘待排序数组’中
	int k = 0;
	for (int i = 0; i < n; i++)
	{
		t = bucket[i].next;
		while (t != NULL)
		{
			datas->data[k++] = t->value;
			t = t->next;
		}
	}

	//TODO: 遍历释放node
	// **
	free(bucket);
}


#endif //_JVT_BUCKET_H_
