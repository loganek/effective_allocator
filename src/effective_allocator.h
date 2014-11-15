#include "logger.h"

#include <cassert>
#include <memory>
#include <vector>
#include <limits>

template<class T, size_t SIZE>
struct effective_allocator
{
public:
	struct _node
	{
		char* data;
		char* current;
		char* end;
		int alloc_cnt = 0;

		void init (int size)
		{
			LOG_D(this, "initialize node: ", sizeof(T), "*", size);
			auto s = sizeof (T) * size;
			data = (char*)operator new(s);
			current = data;
			end = data + s + 1;
			LOG_D(this, "new data:", (void*)data);
		}
	};

	std::vector<_node> nodes;
public:
	typedef std::size_t size_type;
	typedef T value_type;
	typedef std::ptrdiff_t difference_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	template<typename U>
	struct rebind
	{
		typedef effective_allocator<U, SIZE> other;
	};

	inline explicit effective_allocator()
	{
		LOG_D(this, "constructor");
		nodes.push_back(_node());
		nodes.back().init(SIZE);
	}

	effective_allocator(const effective_allocator& a)
	{
		LOG_D(this, "copy constructor", &a);
	}

	template<typename U>
	effective_allocator(const effective_allocator<U, SIZE>& a)
	{
		LOG_D(this, "rebind constructor", &a);
		for (auto n : a.nodes)
		{
			nodes.push_back(_node());
//			/nodes.back().init((n.end - n.data - 1)/sizeof(U));
			nodes.back().init(SIZE);
			nodes.back().alloc_cnt = n.alloc_cnt;
		}
	}

	inline ~effective_allocator()
	{
		for (auto n : nodes)
		{
			::operator delete(n.data);
			LOG_D(this, "Removing", (void*)n.data, "in destructor");
		}
	}

	pointer address(reference x) const noexcept
	{ return std::addressof(x); }
	const_pointer address(const_reference x) const noexcept
	{ return std::addressof(x); }

	size_type max_size() const noexcept
	{ return size_type(-1) / sizeof(T); }

	void construct(pointer p, const T& t) { new(p) T(t); }
	void destroy(pointer p) { p->~T(); }

	inline pointer allocate(size_type n, typename std::allocator<void>::const_pointer p = 0)
	{
		if (n > this->max_size())
			std::__throw_bad_alloc();

		void* ptr = nullptr;
		LOG_D(this, "Allocation request, count:", n, "element sizeof: ", sizeof(T), "summary:", n*sizeof(T))
		if (nodes.back().current + sizeof(T) * n < nodes.back().end)
		{
			LOG_D(this, "used existing memory");
			ptr = nodes.back().current;
			nodes.back().current += sizeof(T)*n;
		}
		else
		{
			LOG_D(this, "allocate new memory");
			nodes.push_back(_node());
			nodes.back().init(SIZE > n ? SIZE : n); // todo std::max
			nodes.back().current += sizeof (T)*n;
			ptr = nodes.back().data;
		}
		nodes.back().alloc_cnt += n;

		LOG_D(this, "acquired memory", ptr);

		return reinterpret_cast<pointer>(ptr);
	}

	inline void deallocate(pointer p, size_type cnt)
	{
		return;
		LOG_D(this, "Deallocating pointer", p, "size", cnt);
		for (auto i = nodes.begin(); i != nodes.end(); ++i)
		{
			if ((void*)p < i->end && (void*)p >= i->data)
			{
				LOG_D(this, "Found pointer's node");
				i->alloc_cnt -= cnt;
				assert(i->alloc_cnt >= 0);
				LOG_D(this, "Node's allocation counter:", i->alloc_cnt);
				if (i->alloc_cnt == 0)
				{
					std::swap(*i, nodes.back());
					i = nodes.end();
					--i;
					if (nodes.size() > 1 && nodes.end()[-2].alloc_cnt == 0)
					{
						::operator delete(i->data);
						nodes.erase(i);
						LOG_D(this, "Completely free, nodes left:", nodes.size());
					}
					else
					{
						i->current = i->data;
						LOG_D(this, "Moved current pointer");
					}
				}
				break;
			}
		}
	}
};
