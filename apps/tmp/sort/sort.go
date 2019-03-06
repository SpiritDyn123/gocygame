package main

import "fmt"

func quick_sort(arr []int) {
	arr_len := len(arr)
	if arr_len < 2 {
		return
	}


	b_i := 1
	e_i := arr_len - 1
	s_v := arr[0]
	for {
		for b_i != e_i && arr[e_i] >= s_v  {
			e_i--
		}
		for b_i != e_i && arr[b_i] <= s_v {
			b_i++
		}

		if b_i != e_i {
			arr[b_i], arr[e_i] = arr[e_i], arr[b_i]
		} else {
			break
		}
	}

	if arr[b_i] < s_v {
		arr[0], arr[b_i] = arr[b_i], arr[0]
	}
	quick_sort(arr[:b_i])
	quick_sort(arr[b_i+1:])
}

func bubble_sort(arr []int) {
	arr_len := len(arr)
	if arr_len < 2 {
		return
	}

	for i := 0;i < arr_len - 1;i++ {
		flag := true
		for j := 0;j < arr_len - 1 - i;j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
				flag = false
			}
		}

		if flag {
			break
		}
	}
}

func choice_sort(arr []int) {
	arr_len := len(arr)
	if arr_len < 2 {
		return
	}

	tmp_index := 0
	for i := 0; i < arr_len - 1;i++ {
		tmp_index = i
		for j := i+1;j < arr_len;j++ {
			if arr[j] < arr[tmp_index] {
				tmp_index = j
			}
		}
		if tmp_index != i {
			arr[tmp_index], arr[i] = arr[i], arr[tmp_index]
		}
	}
	
}

func direct_insert_sort(arr []int) {
	arr_len := len(arr)
	if arr_len < 2 {
		return
	}

	tmp_v := 0
	j := 0
	for i := 1;i < arr_len;i++ {
		tmp_v = arr[i]
		for j =i;j > 0;j-- {
			if arr[j-1] < tmp_v {
				break
			}
			arr[j] = arr[j-1]
		}

		if j != i {
			arr[j] = tmp_v
		}
	}
}

//二分查找
func binary_search(arr []int, find_v int) (pos int) {
	pos = -1
	arr_len := len(arr)
	if arr_len == 0 {
		return
	}

	left, right := 0, arr_len
	for {
		if left >= right || left == arr_len -1 {//千万要注意这个向下取整的，可能死循环，如果find_v大于最大值，需要特殊处理
			break
		}

		mid := (left + right) / 2 //千万要注意这个向下取整的，可能死循环，如果find_v大于最大值，需要特殊处理
		if arr[mid] == find_v {
			pos = mid
			return
		} else if arr[mid] > find_v {
			right = mid
		} else {
			left = mid
		}
	}

	return
}

func heap_sort() {

}

func main() {
	arr := []int{1,6,2,5,4 ,3 , 9  ,7 ,10 , 8}
	direct_insert_sort(arr)
	fmt.Println(binary_search(arr, 10))
}
