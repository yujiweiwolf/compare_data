上海的新数据，报单与成交合并的新结构， match_no与老版本不一样， 只能目视比较

# v1.0.22 (2024-11-15)
* mem中不只一天的数据。如果新老数据都是mem, 只比较今天的数据

# v1.0.13 (2024-07-05)
* 修复成交sum_volume的累计问题， 暂不比较sum_amout

# v1.0.10 (2024-06-27)
* 检查盘口，根据最后一笔Tick数据， 判断逐笔成交中的match_volume和match_volume

# v1.0.7 (2024-03-12)
* tick中的new_amount与sum_amount误差放大到10之内

# v1.0.1 (2024-03-04)
* init version
