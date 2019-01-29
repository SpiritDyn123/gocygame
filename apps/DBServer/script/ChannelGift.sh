#!/bin/bash
redis_ip="172.19.68.57"
redis_port=19000


python ChannelGift.py -s $redis_ip -p $redis_port -f CSVChannelGift.csv
