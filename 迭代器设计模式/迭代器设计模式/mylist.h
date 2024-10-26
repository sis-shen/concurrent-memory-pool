#include <iostream>


template<typename T, typename Ptr, typename Ref>
class __base_iterator
{
public:
	__base_iterator() = default;

	__base_iterator(const __base_iterator<T, T*, T&>& it){};//要支持普通迭代器构造const迭代器

	virtual __base_iterator<T, Ptr, Ref>& operator++() = 0;//向前遍历

	//不是所有容器都支持反向迭代器，所以这里根据具体情况声明函数
	//__base_iterator<T>& operator--() = 0; 

	virtual Ptr operator->() = 0;//重载->操作，使其部分实现指针功能

	virtual Ref operator*() = 0;//重载解引用，部分实现指针功能

	virtual bool operator!=(const __base_iterator<T,Ptr,Ref>&)const = 0;//应当支持迭代器的不等比较
};

template<typename ValueType>
struct ListNode
{
	ListNode(const ValueType& val = ValueType()) :_val(val), _next(nullptr), _prev(nullptr) {}
	ValueType _val;
	ListNode<ValueType>* _next;
	ListNode<ValueType>* _prev;
};


template<typename T,typename Ptr,typename Ref>
class __list_iterator :public __base_iterator<T,Ptr,Ref>
{
	typedef __list_iterator<T, Ptr, Ref> Self;
	typedef __list_iterator<T, T*, T&> iterator;
public:
	__list_iterator(ListNode<T>* ptr) :_ptr(ptr) {}

	__list_iterator(const iterator& it) :_ptr(it._ptr) {}

	__base_iterator<T, Ptr, Ref>& operator++() override
	{
		_ptr = _ptr->_next;
		return *this;
	}

	Ptr operator->() override
	{
		return &(_ptr->_val);
	}

	Ref operator*() override
	{
		return _ptr->_val;
	}

	bool operator!=(const __base_iterator<T,Ptr,Ref>&it)const 
	{
		const Self* pit = dynamic_cast<const Self*>(&it);//指针的(父类子类间)动态转换
		if (pit)
		{
			return _ptr != pit->_ptr;
		}
		return true;//类型不匹配,认为不相等
	}

private:
	ListNode<T>* _ptr;
};



template<typename Type>
class List
{
	typedef ListNode<Type> Node;//简化代码

public:
	typedef __list_iterator<Type, Type*, Type&> iterator;
	typedef __list_iterator<Type, const Type*, const Type&> const_iterator;
public:
	List() :_head(new Node) { _head->_next = _head; _head->_prev = _head; }

	iterator begin()
	{
		return _head->_next;
	}

	iterator end()
	{
		return _head;
	}

	const_iterator begin() const
	{
		return _head->_next;
	}

	const_iterator end() const
	{
		return _head;
	}

	void push_front(const Type& val)
	{
		Node* newnode = new Node(val);
		Node* prev = _head;
		Node* next = _head->_next;

		prev->_next = newnode;
		next->_prev = newnode;

		newnode->_prev = prev;
		newnode->_next = next;
	}

	void push_back(const Type& val)
	{
		Node* newnode = new Node(val);
		Node* next = _head;
		Node* prev = _head->_prev;

		prev->_next = newnode;
		next->_prev = newnode;

		newnode->_prev = prev;
		newnode->_next = next;
	}

	void inorder()
	{
		Node* cur = _head->_next;
		while (cur != _head)
		{
			std::cout << cur->_val << "->";
			cur = cur->_next;
		}
		std::cout << "_head\n";
	}
private:
	Node* _head;
};