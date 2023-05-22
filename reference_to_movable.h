#ifndef BYES_LIVE_REF_H
#define BYES_LIVE_REF_H

#include <type_traits>
#include <iostream>
namespace byes {
	
	template<typename Referenced>
	class ReferencedMovable;

	template<typename Referenced>
	class ReferenceToMovable;

	template<typename NoQualifierReferenced>
	class ReferenceToMovableBase
	{
		friend class ReferencedMovable<NoQualifierReferenced>;

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
			
			if (referenced_ptr_ && referenced_ptr_->reference_node_ == this)
			{
				if (prev_)
				{
					referenced_ptr_->reference_node_ = prev_;
				}
				else
				{
					referenced_ptr_->reference_node_ = next_;
				}
			}

			referenced_ptr_ = nullptr;
			prev_ = nullptr;
			next_ = nullptr;
		}

		void Link(NoQualifierReferenced* referenced_ptr)
		{
			if (referenced_ptr)
			{
				referenced_ptr_ = referenced_ptr;

				if (referenced_ptr_->reference_node_)
				{
					next_ = referenced_ptr_->reference_node_->next_;
					if (next_)
					{
						next_->prev_ = this;
					}

					prev_ = referenced_ptr_->reference_node_;
					referenced_ptr_->reference_node_->next_ = this;
				}
				else
				{
					next_ = nullptr;
					prev_ = nullptr;
					referenced_ptr_->reference_node_ = this;
				}
			}
		}

		void Move(ReferenceToMovableBase& moved_rtm)
		{
			referenced_ptr_ = moved_rtm.referenced_ptr_;
			prev_ = moved_rtm.prev_;
			next_ = moved_rtm.next_;


			if (prev_)
				prev_->next_ = this;
			if (next_)
				next_->prev_ = this;

			if (referenced_ptr_)
			{
				referenced_ptr_->reference_node_ = this;
				moved_rtm.referenced_ptr_ = nullptr;

				moved_rtm.next_ = nullptr;
				moved_rtm.prev_ = nullptr;
			}
		}

		void OnReferencedMovedOrDeleted(NoQualifierReferenced* new_referenced_ptr) noexcept
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
		{
		}
		
		ReferenceToMovableBase(const ReferenceToMovableBase& copied_rtm) noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{
			Link(copied_rtm.referenced_ptr_);
		}
		
		ReferenceToMovableBase(ReferenceToMovableBase&& moved_rtm) noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{
			Move(moved_rtm);
		}

		ReferenceToMovableBase(NoQualifierReferenced& referenced) noexcept :referenced_ptr_(nullptr), prev_(nullptr), next_(nullptr)
		{
			Link(&referenced);
		}

		ReferenceToMovableBase& operator= (const NoQualifierReferenced& copied_rtm)
		{
			Reset();
			LinkToReferenced(copied_rtm.referenced_ptr_);
			return this;
		}

		ReferenceToMovableBase& operator= (ReferenceToMovableBase&& moved_rtm)
		{
			Reset();
			Move(moved_rtm);
			return *this;
		}

		~ReferenceToMovableBase() noexcept
		{
			Reset();
		}

	protected:

		mutable ReferenceToMovableBase* prev_;
		mutable ReferenceToMovableBase* next_;

		NoQualifierReferenced* referenced_ptr_;
	};

	template<typename Referenced>
	class ReferenceToMovable : ReferenceToMovableBase<typename std::remove_cv<Referenced>::type>
	{

	public:

		ReferenceToMovable() = default;

		ReferenceToMovable(Referenced& ref) noexcept : ReferenceToMovableBase<typename std::remove_cv<Referenced>::type>(const_cast<std::remove_cv<Referenced>::type&>(ref))
		{}

		ReferenceToMovable(const ReferenceToMovable&) noexcept = default;
		ReferenceToMovable(ReferenceToMovable&&) noexcept = default;

		ReferenceToMovable& operator= (const ReferenceToMovable& ref) = default;
		ReferenceToMovable& operator= (ReferenceToMovable&& ref) = default;


		Referenced* operator->() const noexcept
		{
			return ReferenceToMovableBase<typename std::remove_cv<Referenced>::type>::referenced_ptr_;
		}

		operator bool() const noexcept
		{
			return ReferenceToMovableBase<typename std::remove_cv<Referenced>::type>::referenced_ptr_ != nullptr;
		}

		operator Referenced& () const { return *ReferenceToMovableBase<typename std::remove_cv<Referenced>::type>::referenced_ptr_; }

		friend class ReferencedMovable<typename std::remove_cv<Referenced>::type>;
	};

	template<typename Referenced>
	class ReferencedMovable
	{
	private:
		ReferenceToMovableBase<Referenced>* reference_node_;
	public:
		ReferencedMovable() noexcept :reference_node_(nullptr) {};
		ReferencedMovable(const ReferencedMovable&) noexcept :reference_node_(nullptr) {};
		ReferencedMovable(ReferencedMovable&& t) noexcept :reference_node_(t.reference_node_) 
		{
			t.reference_node_ = nullptr;
			if (reference_node_)
			{
				reference_node_->OnReferencedMovedOrDeleted(static_cast<Referenced*>(this));
			}
		};

		ReferencedMovable& operator=(const ReferencedMovable&) noexcept { reference_node_ = nullptr; return this; }
		ReferencedMovable& operator=(ReferencedMovable&& t) noexcept
		{
			reference_node_ = t.reference_node_; 
			t.reference_node_ = nullptr;
			if (reference_node_)
			{
				reference_node_->OnReferencedMovedOrDeleted(static_cast<Referenced*>(this));
			}
			return this; 
		}

		friend class ReferenceToMovableBase<Referenced>;

		~ReferencedMovable() noexcept { if (reference_node_) reference_node_->OnReferencedMovedOrDeleted(nullptr); }
	};

	template<typename Referenced>
	using RTM = ReferenceToMovable<Referenced>;

	template<typename Referenced>
	using RM = ReferencedMovable<Referenced>;

}

#endif // BYES_LIVE_REF_H