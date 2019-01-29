#/bin/bash
mysql -uroot -pM@o3L!Ff4 -h10.0.0.18 global -e "select qaid,',', seq,',', ans,',', count(*) from qa_info group by qaid, seq, ans;" -N > test
