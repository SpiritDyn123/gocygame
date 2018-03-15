package timer

import (
	"fmt"
	"sync/atomic"
)

//定义节点
type Node struct {
	data interface{}
	prev *Node
	next *Node
}

func (node *Node ) Delete() {
	if nil == node {
		return
	} else if nil == node.prev { //该元素处于表头，不删除，默认表头不存元素
		return
	} else if nil == node.next { //该元素处于表尾
		node.prev.next = nil
		node.prev = nil
	} else {
		node.next.prev = node.prev
		node.prev.next = node.next
		node.prev = nil
		node.next = nil
	}
}

func (this *Node) InsertHead(node *Node) *Node { //从表头插入
	if nil == this || nil != this.prev { //为空，或者不是表头(表头的prev为空)
		return nil
	} else {
		if nil != this.next {
			this.next.prev = node
			node.next = this.next
		}
		this.next = node
		node.prev = this
	}
	return node
}

func (this *Node) Next() (node *Node) {
	return this.next
}

func (this *Node) Prev() (node *Node) {
	return this.prev
}

func (this *Node) Data() (data interface{}) {
	return this.data
}

func (this *Node) SetData(data interface{}) {
	this.data = data
}

const wheel_cnt uint8 = 5   //时间轮数量5个
var element_cnt_per_wheel = [wheel_cnt]uint32{256, 64, 64, 64, 64}                          //每个时间轮的槽(元素)数量。在 256+64+64+64+64 = 512 个槽中，表示的范围为 2^32
var right_shift_per_wheel = [wheel_cnt]uint32{8, 6, 6, 6, 6}                                //当指针指向当前时间轮最后一位数，再走一位就需要向上进位。每个时间轮进位的时候，使用右移的方式，最快实现进位。这里是每个轮的进位二进制位数
var base_per_wheel = [wheel_cnt]uint32{1, 256, 256 * 64, 256 * 64 * 64, 256 * 64 * 64 * 64} //记录每个时间轮指针当前指向的位置

type TimerHandler interface {
	OnTimer(...interface{})
}

type TimerHandlerFunc func(...interface{})

func(f TimerHandlerFunc) OnTimer(args ...interface{}) {
	f(args...)
}

type WheelTimer interface {
	SetTimer(inteval uint32, keep bool, handler TimerHandler, id uint64, args ...interface{}) (uint64, error)
	DelTimer(id uint64)
	Step()
}

type mtimer struct {
	//Name        string            //定时器名称
	Inteval     uint32            //时间间隔，即以插入该定时器的时间为起点，Inteval秒之后执行回调函数DoSomething()。例如进程插入该定时器的时间是2015-04-05 10:23:00，Inteval=5，则执行DoSomething()的时间就是2015-04-05 10:23:05。
	Left 		uint32
	Keep        bool
	Handler TimerHandler //自定义事件处理函数，需要触发的事件
	Args        []interface{}       //上述函数的输入参数
	Id 		uint64
}

type wheelTimer struct {
	newest [wheel_cnt]uint32 //每个时间轮当前指针所指向的位置
	timewheels [5][]*Node    //定义5个时间轮
	timerPos map[uint64]*Node
	timerId uint64
}

func (wt *wheelTimer) SetTimer(inteval uint32, keep bool, handler TimerHandler, id uint64, args ...interface{}) (timerId uint64, err error) {
	if inteval <= 0 {
		return 0, fmt.Errorf("inteval <= 0")
	}
	var bucket_no uint8 = 0
	var offset uint32 = inteval
	var left uint32 = inteval
	for offset >= element_cnt_per_wheel[bucket_no] { //偏移量大于当前时间轮容量，则需要向高位进位
		offset >>= right_shift_per_wheel[bucket_no] //计算高位的值。偏移量除以低位的进制。比如低位当前是256，则右移8个二进制位，就是除以256，得到的结果是高位的值。
		var tmp uint32 = 1
		if bucket_no == 0 {
			tmp = 0
		}
		left -= base_per_wheel[bucket_no] * (element_cnt_per_wheel[bucket_no] - wt.newest[bucket_no] - tmp)
		bucket_no++
	}

	if offset < 1 {
		return 0, fmt.Errorf("inteval too much")
	}

	if inteval < base_per_wheel[bucket_no]*offset {
		return 0, fmt.Errorf("inteval < base_per_wheel[bucket_no]*offset")
	}

	left -= base_per_wheel[bucket_no] * (offset - 1)
	pos := (wt.newest[bucket_no] + offset) % element_cnt_per_wheel[bucket_no] //通过类似hash的方式，找到在时间轮上的插入位置

	timerId = id
	mt_interval := inteval
	if id == 0 {
		timerId = atomic.AddUint64(&wt.timerId, 1)
	} else {
		if _, ok := wt.timerPos[id];!ok {
			return 0, fmt.Errorf("timer id error")
		}

		mt_interval = wt.timerPos[id].data.(mtimer).Inteval
	}

	node := &Node{}
	node.SetData( mtimer{mt_interval, left, keep,handler, args, timerId})
	//TimerMap[name] = timewheels[bucket_no][pos].InsertHead(node) //插入定时器
	wt.timewheels[bucket_no][pos].InsertHead(node) //插入定时器

	wt.timerPos[timerId] = node

	return
}

func (wt *wheelTimer) DelTimer(id uint64) {
	if node, ok := wt.timerPos[id];ok {
		node.Delete()
	}
}

func (wt *wheelTimer) Step() {
	//var dolist list.List
	{
		//遍历所有桶
		var bucket_no uint8 = 0
		for bucket_no = 0; bucket_no < wheel_cnt; bucket_no++ {
			wt.newest[bucket_no] = (wt.newest[bucket_no] + 1) % element_cnt_per_wheel[bucket_no] //当前指针递增1
			//fmt.Println(newest)
			var head *Node = wt.timewheels[bucket_no][wt.newest[bucket_no]] //返回当前指针指向的槽位置的表头
			var firstElement *Node = head.Next()
			for firstElement != nil { //链表不为空
				if value, ok := firstElement.Data().(mtimer); ok { //如果element里面确实存储了Timer类型的数值，那么ok返回true，否则返回false。
					left := value.Left
					handler := value.Handler
					args := value.Args
					if nil != handler { //有遇到函数为nil的情况，所以这里判断下非nil
						if 0 == bucket_no || 0 == left {
							handler.OnTimer(args...)
							if value.Keep {
								wt.SetTimer(value.Inteval, value.Keep, handler, value.Id, args...) //重新插入计时器
							}
						} else {
							wt.SetTimer(left, value.Keep, handler, value.Id, args...) //重新插入计时器
						}
					}
					firstElement.Delete() //删除定时器
				}
				firstElement = head.Next() //重新定位到链表第一个元素头
			}
			if 0 != wt.newest[bucket_no] { //指针不是0，还未转回到原点，跳出。如果回到原点，则说明转完了一圈，需要向高位进位1，则继续循环入高位步进一步。
				break
			}
		}
	}
}

func CreateWheelTimer() WheelTimer {
	wt := &wheelTimer{
		timerPos:make(map[uint64]*Node),
	}

	var bucket_no uint8 = 0
	for bucket_no = 0; bucket_no < wheel_cnt; bucket_no++ {
		var i uint32 = 0
		for ; i < element_cnt_per_wheel[bucket_no]; i++ {
			wt.timewheels[bucket_no] = append(wt.timewheels[bucket_no], new(Node))
		}
	}

	return wt
}
