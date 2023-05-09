#ifndef BYES_LIVE_REF_H
#define BYES_LIVE_REF_H

#include <type_traits>

namespace byes {
	
	template<typename RawType>
	class ReferencedMovable;

	template<typename RawType>
	class ReferenceToMovable;

	template<typename NoQualifierRawType>
	class ReferenceToMovableBase
	{
		friend class ReferencedMovable<NoQualifierRawType>;

		void Reset()
		{
			if (prev_)
			{
				prev_->next_ = next_;
			}

			if (next_)
			{
				next_->prev_ = prev_;
			}
			
			if (referenced_ptr_ && referenced_ptr_->ref_ == this)
			{
				if (prev_)
				{
					referenced_ptr_->ref_ = prev_;
				}
				else
				{
					referenced_ptr_->ref_ = next_;
				}
			}

			referenced_ptr_ = nullptr;
			prev_ = nullptr;
			next_ = nullptr;
		}

		void LinkToReferenced(NoQualifierRawType* referenced_ptr)
		{
			if (referenced_ptr)
			{
				referenced_ptr_ = referenced_ptr;

				if (referenced_ptr_->ref_)
				{
					next_ = referenced_ptr_->ref_->next_;
					if (next_)
					{
						next_->prev_ = this;
					}

					prev_ = referenced_ptr_->ref_;
					referenced_ptr_->ref_->next_ = this;
				}
				else
				{
					next_ = nullptr;
					prev_ = nullptr;
					referenced_ptr_->ref_ = this;
				}
			}
		}

		void OnReferencedMovedOrDeleted(NoQualifierRawType* new_referenced_ptr) noexcept
		{
			referenced_ptr_ = new_referenced_ptr;

			auto ref = next_;
			while (ref)
			{
				ref->referenced_ptr_ = new_referenced_ptr;

				auto next = ref->next_;

				if (!ref->referenced_ptr_)
				{
					ref->prev_ = nullptr;
					ref->next_ = nullptr;
				}

				ref = next;
			}

			ref = prev_;

			while (ref)
			{
				ref->referenced_ptr_ = new_referenced_ptr;

				auto prev = ref->prev_;

				if (!ref->referenced_ptr_)
				{
					ref->prev_ = nullptr;
					ref->next_ = nullptr;
				}

				ref = prev;
			}

			if (!new_referenced_ptr)
			{
				next_ = nullptr;
				prev_ = nullptr;
			}
		}

	public:

		ReferenceToMovableBase() noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{}

		ReferenceToMovableBase(const NoQualifierRawType& ref) noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{
			LinkToReferenced(const_cast<NoQualifierRawType*>(&ref));
		}

		ReferenceToMovableBase(const ReferenceToMovableBase& ref) noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{
			LinkToReferenced(ref.referenced_ptr_);
		}

		ReferenceToMovableBase(ReferenceToMovableBase&& r) noexcept :referenced_ptr_(r.referenced_ptr_), prev_(r.prev_), next_(r.next_)
		{
			if (prev_)
				prev_->next_ = this;
			if (next_)
				next_->prev_ = this;

			if (referenced_ptr_)
			{
				referenced_ptr_->ref_ = this;
				r.referenced_ptr_ = nullptr;

				r.next_ = nullptr;
				r.prev_ = nullptr;
			}
		}

		ReferenceToMovableBase& operator= (const ReferenceToMovableBase& ref)
		{
			Reset();
			LinkToReferenced(ref.referenced_ptr_);
		}

		ReferenceToMovableBase& operator= (ReferenceToMovableBase&& ref)
		{

			referenced_ptr_ = ref.referenced_ptr_;
			prev_ = ref.prev_;
			next_ = ref.next_;

			if (prev_)
				prev_->next_ = this;
			if (next_)
				next_->prev_ = this;

			if (referenced_ptr_)
			{
				referenced_ptr_->ref_ = this;
				ref.referenced_ptr_ = nullptr;

				ref.next_ = nullptr;
				ref.prev_ = nullptr;
			}

			return *this;
		}

		~ReferenceToMovableBase() noexcept
		{
			Reset();
		}

	protected:

		mutable ReferenceToMovableBase* prev_;
		mutable ReferenceToMovableBase* next_;

		NoQualifierRawType* referenced_ptr_;
	};

	template<typename RawType>
	class ReferenceToMovable : ReferenceToMovableBase<typename std::remove_cv<RawType>::type>
	{

	public:

		ReferenceToMovable() = default;

		ReferenceToMovable(RawType& ref) noexcept : ReferenceToMovableBase<typename std::remove_cv<RawType>::type>(const_cast<std::remove_cv<RawType>::type&>(ref))
		{}

		ReferenceToMovable(const ReferenceToMovable&) = default;
		ReferenceToMovable(ReferenceToMovable&&) = default;

		ReferenceToMovable& operator= (const ReferenceToMovable& ref) = default;
		ReferenceToMovable& operator= (ReferenceToMovable&& ref) = default;


		RawType* operator->() const noexcept
		{
			return ReferenceToMovableBase<typename std::remove_cv<RawType>::type>::referenced_ptr_;
		}

		operator RawType& () const { return *ReferenceToMovableBase<typename std::remove_cv<RawType>::type>::referenced_ptr_; }

		friend class ReferencedMovable<typename std::remove_cv<RawType>::type>;
	};

	template<typename RawType>
	class ReferencedMovable
	{
	private:
		ReferenceToMovableBase<RawType>* ref_;
	public:
		ReferencedMovable() noexcept :ref_(nullptr) {};
		ReferencedMovable(const ReferencedMovable&) noexcept :ref_(nullptr) {};
		ReferencedMovable(ReferencedMovable&& t) noexcept :ref_(t.ref_) 
		{
			t.ref_ = nullptr;
			if (ref_)
			{
				ref_->OnReferencedMovedOrDeleted(static_cast<RawType*>(this));
			}
		};

		ReferencedMovable& operator=(const ReferencedMovable&) noexcept { ref_ = nullptr; return this; }
		ReferencedMovable& operator=(ReferencedMovable&& t) noexcept
		{
			ref_ = t.ref_; 
			t.ref_ = nullptr;
			if (ref_)
			{
				ref_->OnReferencedMovedOrDeleted(static_cast<RawType*>(this));
			}
			return this; 
		}

		friend class ReferenceToMovableBase<RawType>;

		~ReferencedMovable() noexcept { if (ref_) ref_->OnReferencedMovedOrDeleted(nullptr); }
	};

	template<typename RawType>
	using RTM = ReferenceToMovable<RawType>;

	template<typename RawType>
	using RM = ReferencedMovable<RawType>;

}

#endif // BYES_LIVE_REF_H