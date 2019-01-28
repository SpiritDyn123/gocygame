package utils

import (
	//	"hash"
	"math"
	"sort"
	"hash/crc32"
	"fmt"
)

const (
	//DefaultVirualSpots default virual spots
	Default_Consistent_hash_virtual_sport  = 40
)

type consistentHashNode struct {
	nodeKey   string
	spotValue uint32
}

type nodesArray []consistentHashNode

func (p nodesArray) Len() int           { return len(p) }
func (p nodesArray) Less(i, j int) bool { return p[i].spotValue < p[j].spotValue }
func (p nodesArray) Swap(i, j int)      { p[i], p[j] = p[j], p[i] }
func (p nodesArray) Sort()              { sort.Sort(p) }

//HashRing store nodes and weigths
type ConsistentHashRing struct {
	virualSpots int
	nodes       nodesArray
	weightNodes     map[string]int
}

//NewHashRing create a hash ring with virual spots
func NewConsistentHashRing(spots int) *ConsistentHashRing {
	if spots == 0 {
		spots = Default_Consistent_hash_virtual_sport
	}

	h := &ConsistentHashRing{
		virualSpots: spots,
		weightNodes:     make(map[string]int),
	}
	return h
}

//AddNodes add nodes to hash ring
func (h *ConsistentHashRing) AddNodes(nodeWeight map[string]int) {
	for nodeKey, w := range nodeWeight {
		h.weightNodes[nodeKey] = w
	}
	h.generate()
}

//AddNode add node to hash ring
func (h *ConsistentHashRing) AddNode(nodeKey string, weight int) {
	h.weightNodes[nodeKey] = weight
	h.generate()
}

//RemoveNode remove node
func (h *ConsistentHashRing) RemoveNode(nodeKey string) {
	delete(h.weightNodes, nodeKey)
	h.generate()
}

//UpdateNode update node with weight
func (h *ConsistentHashRing) UpdateNode(nodeKey string, weight int) {
	h.weightNodes[nodeKey] = weight
	h.generate()
}

func (h *ConsistentHashRing) generate() {
	var totalW int
	for _, w := range h.weightNodes {
		totalW += w
	}

	totalVirtualSpots := h.virualSpots * len(h.weightNodes)
	h.nodes = nodesArray{}

	for nodeKey, w := range h.weightNodes {
		spots := int(math.Floor(float64(w) / float64(totalW) * float64(totalVirtualSpots))) //计算出应该分布几个虚节点
		for i := 1; i <= spots; i++ {
			sport_key := genHashKey(fmt.Sprintf("%s#%d", nodeKey, i))
			n := consistentHashNode{
				nodeKey:   nodeKey,
				spotValue: sport_key,
			}

			//检查时是否碰撞了
			for _, e_node := range h.nodes {
				if e_node.spotValue == sport_key && e_node.nodeKey != nodeKey{
					panic(fmt.Sprintf("ConsistentHashRing generate has key impact key:%s key2:%s", e_node.nodeKey, nodeKey))
				}
			}

			h.nodes = append(h.nodes, n)
		}
	}

	h.nodes.Sort()
}

func genHashKey(stringKey string) uint32 {
	return crc32.ChecksumIEEE([]byte(stringKey))
}

//GetNode get node with key
func (h *ConsistentHashRing) GetNode(s string) string {
	if len(h.nodes) == 0 {
		return ""
	}

	sport_key := genHashKey(s)
	i := sort.Search(len(h.nodes), func(i int) bool { return h.nodes[i].spotValue >= sport_key })

	if i == len(h.nodes) {
		i = 0
	}

	return h.nodes[i].nodeKey
}