package main

// 修改映射
// 在映射 m 中插入或修改元素：
// m[key] = elem
// 获取元素：
// elem = m[key]
// 删除元素：
// delete(m, key)
// 通过双赋值检测某个键是否存在：
// elem, ok = m[key]
// 若 key 在 m 中，ok 为 true ；否则，ok 为 false。
// 若 key 不在映射中，那么 elem 是该映射元素类型的零值。

import "fmt"

func main() {
	m := make(map[string]int)

	m["Answer"] = 42
	fmt.Println("The value:", m["Answer"])

	m["Answer"] = 48
	fmt.Println("The value:", m["Answer"])

	delete(m, "Answer")
	fmt.Println("The value:", m["Answer"])

	v, ok := m["Answer"]
	fmt.Println("The value:", v, "Present?", ok)
}