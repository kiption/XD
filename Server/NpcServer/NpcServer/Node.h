#pragma once

struct Node
{
	int index; // ����� �ε���
	float heuristic; // ��ǥ���������� �޸���ƽ ��
	float cost; // ����������� ���� �������� ���
	Node* parent; // �θ� ���

	Node(int idx, float h, float c, Node* p) : index(idx), heuristic(h), cost(c), parent(p) {}

	// ���� �޸���ƽ ���� ��ģ �켱���� ť�� ���� ������ �����ε�
	bool operator<(const Node& other) const {
		return (cost + heuristic) > (other.cost + other.heuristic);
	}
};