#pragma once
template <class T>
class MTQueue
{
public:
	MTQueue();
	MTQueue(size_t nSize, size_t nCapacity);
	~MTQueue();

	T* Resize(size_t nNewSize);
	size_t Size();
	size_t Capacity();
	void Clear();

	T* operator*();

private:
	T* m_pData;
	size_t m_nSize;
	size_t m_nCapacity;

	T* Allocate();
	void Deallocate();
};

template <class T>
MTQueue<T>::MTQueue() :
	m_pData(nullptr),
	m_nSize(0),
	m_nCapacity(0)
{}

template <class T>
MTQueue<T>::MTQueue(size_t nSize, size_t nCapacity) :
	m_nSize(nSize),
	m_nCapacity(nCapacity)
{
	m_pData = Allocate();
}

template <class T>
MTQueue<T>::~MTQueue()
{
	Deallocate();
}

// Resize and return pointer to position of previous capacity
template <class T>
T* MTQueue<T>::Resize(size_t nNewSize)
{
	m_nSize = m_nCapacity;
	m_nCapacity = nNewSize;

	T* newData = Allocate();
	if (m_pData)
		memcpy(newData, m_pData, m_nCapacity);

	Deallocate();
	m_pData = newData;
	return m_pData + m_nSize;
}

template<class T>
void MTQueue<T>::Clear()
{
	Deallocate();
	m_nSize = 0;
	m_nCapacity = 0;
	m_pData = nullptr;
}

template<class T>
size_t MTQueue<T>::Capacity()
{
	return m_nCapacity;
}

template<class T>
size_t MTQueue<T>::Size()
{
	return m_nSize;
}

template<class T>
T* MTQueue<T>::operator*()
{
	return m_pData;
}

template <class T>
T* MTQueue<T>::Allocate()
{
	return new T[m_nCapacity];
}

template <class T>
void MTQueue<T>::Deallocate()
{
	if (m_pData)
	{
		delete m_pData;
	}
}
