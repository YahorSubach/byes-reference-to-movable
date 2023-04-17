#ifndef BYES_LIVE_REF_H
#define BYES_LIVE_REF_H


namespace byes {
	



	template<typename RawType>
	class LiveRefTarget;

	template<typename RawType>
	class LiveRef
	{
	private:
		RawType* ref_ptr_;
		mutable LiveRef* prev_;
		mutable LiveRef* next_;
		mutable bool valid_;

		template<typename T>
		struct RemoveConst
		{
			typedef Type T;
		};

		template<typename T>
		struct RemoveConst<const T>
		{
			typedef Type T;
		};

		Set();
		Reset(): ref_ptr_ = {}

	public:
		LiveRef(LiveRefTarget<RemoveConst<RawType>>& ref):ref_ptr_(&ref), prev_(nullptr), next_(nullptr), valid_(true)
		{}

		friend class LiveRefTarget<RemoveConst<RawType>>;
	};

	template<typename RawType>
	class LiveRefTarget
	{
	private:
		LiveRef<LiveRefTarget>* ref_;
	public:
		LiveRefTarget() :ref_(nullptr) {};


		friend class LiveRef<RawType>;
		friend class LiveRef<const RawType>;
	};

}

#endif // BYES_LIVE_REF_H