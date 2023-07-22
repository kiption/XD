#pragma once

struct Node
{
	int index; // 노드의 인덱스
	float heuristic; // 목표지점까지의 휴리스틱 값
	float cost; // 출발지점부터 현재 노드까지의 비용
	Node* parent; // 부모 노드

	Node(int idx, float h, float c, Node* p) : index(idx), heuristic(h), cost(c), parent(p) {}

	// 비용과 휴리스틱 값을 합친 우선순위 큐를 위한 연산자 오버로딩
	bool operator<(const Node& other) const {
		return (cost + heuristic) > (other.cost + other.heuristic);
	}
};