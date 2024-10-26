#include <iostream>


template<typename T, typename Ptr, typename Ref>
class __base_iterator
{
public:
	__base_iterator() = default;

	__base_iterator(const __base_iterator<T, T*, T&>& it){};//Ҫ֧����ͨ����������const������

	virtual __base_iterator<T, Ptr, Ref>& operator++() = 0;//��ǰ����

	//��������������֧�ַ��������������������ݾ��������������
	//__base_iterator<T>& operator--() = 0; 

	virtual Ptr operator->() = 0;//����->������ʹ�䲿��ʵ��ָ�빦��

	virtual Ref operator*() = 0;//���ؽ����ã�����ʵ��ָ�빦��

	virtual bool operator!=(const __base_iterator<T,Ptr,Ref>&)const = 0;//Ӧ��֧�ֵ������Ĳ��ȱȽ�
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
		const Self* pit = dynamic_cast<const Self*>(&it);//ָ���(���������)��̬ת��
		if (pit)
		{
			return _ptr != pit->_ptr;
		}
		return true;//���Ͳ�ƥ��,��Ϊ�����
	}

private:
	ListNode<T>* _ptr;
};



template<typename Type>
class List
{
	typedef ListNode<Type> Node;//�򻯴���

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