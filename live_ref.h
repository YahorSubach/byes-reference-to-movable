#ifndef BYES_LIVE_REF_H
#define BYES_LIVE_REF_H

#include <type_traits>

namespace byes {
	



	template<typename RawType>
	class LiveRefTarget;

	template<typename NoQualifierRawType>
	class LiveRefBase
	{
		friend class LiveRefTarget<NoQualifierRawType>;

		void Set(NoQualifierRawType* ref_ptr)
		{
			ref_ptr_ = ref_ptr;

			auto ref = next_;
			while (ref)
			{
				ref->ref_ptr_ = ref_ptr;

				auto next = ref->next_;

				if (!ref->ref_ptr_)
				{
					ref->prev_ = nullptr;
					ref->next_ = nullptr;
				}

				ref = next;
			}

			ref = prev_;

			while (ref)
			{
				ref->ref_ptr_ = ref_ptr;

				auto prev = ref->prev_;

				if (!ref->ref_ptr_)
				{
					ref->prev_ = nullptr;
					ref->next_ = nullptr;
				}

				ref = prev;
			}

			if (!ref_ptr)
			{
				next_ = nullptr;
				prev_ = nullptr;
			}
		}

		void Reset() { Set(nullptr); }
	public:
		LiveRefBase(NoQualifierRawType& ref) :ref_ptr_(&ref), prev_(nullptr), next_(nullptr)
		{
			if (ref_ptr_->ref_)
			{
				if (ref_ptr_->ref_->next_)
				{
					next_ = ref_ptr_->ref_->next_;
					ref_ptr_->ref_->next_->prev_ = this;
				}

				ref_ptr_->ref_->next_ = this;

				prev_ = ref_ptr_->ref_;
			}
			else
			{
				ref_ptr_->ref_ = this;
			}
		}

		LiveRefBase(const LiveRefBase& r):ref_ptr_(r.ref_ptr_), prev_(nullptr), next_(nullptr)
		{
			if (r.ref_ptr_)
			{
				r.next_ = this;
				prev_ = const_cast<LiveRefBase*>(&r);
			}
		}

		LiveRefBase(LiveRefBase&& r) :ref_ptr_(r.ref_ptr_), prev_(r.prev_), next_(r.next_)
		{
			if (prev_)
				prev_->next_ = this;
			if (next_)
				next_->prev_ = this;
		}

	protected:

		mutable LiveRefBase* prev_;
		mutable LiveRefBase* next_;

		NoQualifierRawType* ref_ptr_;
	};

	template<typename RawType>
	class LiveRef: LiveRefBase<typename std::remove_cv<RawType>::type>
	{

	public:
		LiveRef(RawType& ref): LiveRefBase<typename std::remove_cv<RawType>::type>(const_cast<std::remove_cv<RawType>::type&>(ref))
		{}


		RawType* operator->() const
		{
			return LiveRefBase::ref_ptr_;
		}

		RawType* operator->()
		{
			return LiveRefBase::ref_ptr_;
		}

		friend class LiveRefTarget<typename std::remove_cv<RawType>::type>;
	};

	template<typename RawType>
	class LiveRefTarget
	{
	private:
		mutable LiveRefBase<RawType>* ref_;
	public:
		LiveRefTarget() :ref_(nullptr) {};
		LiveRefTarget(const LiveRefTarget&) :ref_(nullptr) {};
		LiveRefTarget(LiveRefTarget&& t) :ref_(t.ref_) 
		{
			t.ref_ = nullptr;
			if (ref_)
			{
				ref_->Set(this);
			}
		};

		LiveRefTarget& operator=(const LiveRefTarget&) { ref_ = nullptr; return this; }
		LiveRefTarget& operator=(LiveRefTarget&& t) { ref_ = t.ref_; t.ref_ = nullptr; return this; }

		friend class LiveRefBase<RawType>;

		~LiveRefTarget() { if (ref_) ref_->Reset(); }
	};

}

#endif // BYES_LIVE_REF_H