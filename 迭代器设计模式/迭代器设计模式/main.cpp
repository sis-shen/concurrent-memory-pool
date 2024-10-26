#include "mylist.h"

int main()
{
	List<int> mylst;
	for (int i = 0; i < 10; ++i)
	{
		mylst.push_back(i);
	}

	for (auto e : mylst)
	{
		std::cout << e << "->";
	}


	std::cout << "\n";

	mylst.inorder();
	return 0;
}