#ifndef ENJON_CONTAINERS_H
#define ENJON_CONTAINERS_H

#include "System/Types.h"

#include <queue>
#include <vector>
#include <unordered_set>
#include <utility>
#include <functional>
#include <iostream>
#include <assert.h>

namespace Enjon 
{ 
	template <typename T, typename K>
	struct HeapItem
	{
		HeapItem(){}
		HeapItem(T t, K k)
		{
			Item = t;
			PriorityValue = k;
		}

		T Item;
		K PriorityValue;
		int32 HeapIndex;
	};

	template <typename T, typename K>
	struct Heap
	{
		std::vector<HeapItem<T, K>> Items;
		uint32 CurrentItemCount;
		int32 (*Compare)(HeapItem<T, K>*, HeapItem<T, K>*);

		Heap(int32 MaxHeapSize, int32 (*Compare)(HeapItem<T, K>*, HeapItem<T, K>*))
		{
			Items.resize(MaxHeapSize);
			this->Compare = Compare;
			CurrentItemCount = 0;
		}

		inline uint32 Count()
		{
			return CurrentItemCount;
		}

		inline void Add(HeapItem<T, K> Item)
		{
			Item.HeapIndex = CurrentItemCount;
			Items.at(CurrentItemCount) = Item;
			SortUp(&Items.at(CurrentItemCount));
			CurrentItemCount++;
		}

		HeapItem<T, K> Pop()
		{
			auto FirstItem = Items.at(0);
			if (CurrentItemCount > 0) CurrentItemCount--;
			Items.at(0) = Items.at(CurrentItemCount);
			Items.at(0).HeapIndex = 0;
			SortDown(&Items.at(0));
			return FirstItem;
		}

		inline void SortDown(HeapItem<T, K>* Item)
		{
			auto CurrentItem = Item;

			while (true)
			{
				uint32 ChildIndexLeft 	= CurrentItem->HeapIndex * 2 + 1;
				uint32 ChildIndexRight 	= CurrentItem->HeapIndex * 2 + 2;	
				uint32 SwapIndex 		= 0;

				if (ChildIndexLeft < CurrentItemCount)
				{
					SwapIndex = ChildIndexLeft;

					if (ChildIndexRight < CurrentItemCount)
					{
						if (Compare(&Items.at(ChildIndexLeft), &Items.at(ChildIndexRight)) > 0)
						{
							SwapIndex = ChildIndexRight;	
						}
					}

					if (Compare(CurrentItem, &Items.at(SwapIndex)) > 0)
					{
						auto PreviousIndex = SwapIndex;

						Swap(CurrentItem, &Items.at(SwapIndex));

						CurrentItem = &Items.at(SwapIndex);
					}

					else
					{
						return;
					}

				}

				else
				{
					return;
				}
			}
		}	

		inline void SortUp(HeapItem<T, K>* Item)
		{
			// Only item in heap, so return
			if (!Item->HeapIndex) return;

			auto CurrentItem = Item;

			int32 ParentIndex = (CurrentItem->HeapIndex - 1) / 2;

			while (true)
			{
				auto ParentItem = &Items.at(ParentIndex);
				if (Compare(CurrentItem, ParentItem) < 0)
				{
					Swap(CurrentItem, ParentItem);
				}
				else
				{
					break;
				}

				if (ParentIndex <= 0) break;
				else  
				{
					CurrentItem = ParentItem;
					ParentIndex = (CurrentItem->HeapIndex - 1) / 2;
				}
			}
		}

		inline void Swap(HeapItem<T, K>* A, HeapItem<T, K>* B)
		{
			auto IndexA = A->HeapIndex;
			auto IndexB = B->HeapIndex;
			auto Temp = *A;
			*A = *B;
			*B = Temp;
			A->HeapIndex = IndexA;
			B->HeapIndex = IndexB;
		}


	};	

	const u32 INVALID_SLOT_HANDLE = std::numeric_limits< u32 >::max( );

	template <typename T>
	class slot_array
	{
		public:
			slot_array( )
			{
				clear( );
			}

			class handle
			{
				public:
					friend slot_array< T >;

					handle( ) = default;

					inline u32 get_id( )
					{
						return mhandleID;
					}

					inline const slot_array< T >* get_array( )
					{
						return mslot_array;
					} 

					inline bool is_valid( )
					{
						return mslot_array->is_valid( *this );
					}

					inline bool operator !( )
					{
						return mhandleID != INVALID_SLOT_HANDLE;
					}

					inline const T* get( ) const
					{
						return mslot_array->get_ptr( *this );
					} 

					inline T* get_raw_ptr( )
					{
						return const_cast< handle* >( this )->mslot_array->get_ptr( *this );
					}

					inline const T* operator ->( ) const
					{
						return mslot_array->get_const_ptr( *this );
					}

					inline T* operator ->( )
					{
						return ( mslot_array )->get_ptr( *this );
					}

					inline T& operator *( )
					{
						return ( mslot_array )->get( *this );
					}

					// DON'T CALL YOURSELF
					handle( const u32& handleID, const slot_array< T >* slot_array )
						: mhandleID( handleID ), mslot_array( slot_array )
					{
					} 

				protected:
					u32 mhandleID = INVALID_SLOT_HANDLE;
					const slot_array< T >* mslot_array = nullptr; 
			}; 

			inline usize size( )
			{
				return mData.size( );
			}

			inline T* raw_data_array( )
			{
				return mData.data( );
			}

			inline void reserve( const usize& sz )
			{
				mData.reserve( sz );
			}

			inline bool is_valid( const handle& res ) const
			{ 
				return ( res.mhandleID < mhandleIndices.size() && mhandleIndices[ res.mhandleID ] != INVALID_SLOT_HANDLE );
			}

			inline handle emplace( )
			{
				// Here's the tricky part. Want to push back a new index? Do I store a free list of indices? Do I iterate to find a free index? ( ideally would not do that last bit )	
				// Free list stack? So you push a free index onto the stack and then pop off to get the newest available index? 
				u32 freeIdx = find_next_available_index( );

				// Allocate new resource					
				mData.emplace_back( );

				// Push back new indirection index
				mReverseIndirectionIndices.push_back( freeIdx );

				// This gets the available index in the indirection list of indices, not in the actual resource array ( the reason being that the resource array can have its contents shifted around when adding / removing items )
				// If the index is the last item in the list, then push that on to grow the array
				mhandleIndices[ freeIdx ] = mData.size( ) - 1;

				return { freeIdx, this };

			}

			inline handle insert( const T& v )
			{
				// Here's the tricky part. Want to push back a new index? Do I store a free list of indices? Do I iterate to find a free index? ( ideally would not do that last bit )	
				// Free list stack? So you push a free index onto the stack and then pop off to get the newest available index? 
				u32 freeIdx = find_next_available_index( );

				// Allocate new resource					
				mData.push_back( v );

				// Push back new indirection index
				mReverseIndirectionIndices.push_back( freeIdx );

				// This gets the available index in the indirection list of indices, not in the actual resource array ( the reason being that the resource array can have its contents shifted around when adding / removing items )
				// If the index is the last item in the list, then push that on to grow the array
				mhandleIndices[ freeIdx ] = mData.size( ) - 1;

				return { freeIdx, this };
			}

			inline void erase( const handle& res )
			{
				const u32 res_id = res.mhandleID;

				assert( res_id < mhandleIndices.size( ) );

				// Need to grab the actual resource index from the handle's indirection index
				u32 idx = mhandleIndices[ res_id ];

				// If the index and generation don't match, then we have an invalid handle ( or just use INVALID_handle ) ? Not sure here...  
				assert( idx != INVALID_SLOT_HANDLE );

				// Set handle indirection index to invalid
				mhandleIndices[ res_id ] = INVALID_SLOT_HANDLE;

				// Need to pop and swap data			
				if ( mData.size( ) > 1 )
				{
					// Swap index and back


					std::iter_swap( mData.begin( ) + idx, mData.end( ) - 1 );
					std::iter_swap( mReverseIndirectionIndices.begin( ) + idx, mReverseIndirectionIndices.end( ) - 1 );
				}

				// Update swapped indirection index
				if ( !mData.empty( ) )
				{
					mhandleIndices[ mReverseIndirectionIndices[ idx ] ] = idx;

					// Pop data and reverse indices after use
					mData.pop_back( );
					mReverseIndirectionIndices.pop_back( );
				}

				// Push onto free list for handle
				mIndexFreeList.push_back( res_id );
			}

			inline T& get( const handle& res ) const
			{
				// Something like this? Allocating a new resource would look for the next available index? Or would just push a new index onto stack?
				return mData[ mhandleIndices[ res.mhandleID ] ];
			}

			inline T* get_ptr( const handle& res ) const
			{
				return &mData[ mhandleIndices[ res.mhandleID ] ];
			}

			inline const T* get_const_ptr( const handle& res ) const
			{
				return &mData[ mhandleIndices[ res.mhandleID ] ];
			}

			inline void clear( )
			{
				mData.clear( );
				mIndexFreeList.clear( );
				mhandleIndices.clear( );
				mReverseIndirectionIndices.clear( );
			}

			inline std::vector< T >* data( )
			{
				return &mData;
			}

		private:

			u32 find_next_available_index( )
			{
				// If stack is empty, then simply return the size of the index array, which will be the back index after growing
				if ( mIndexFreeList.empty( ) )
				{
					// Grow the array by 1
					mhandleIndices.push_back( 0 );
					// Return the last index
					return mhandleIndices.size( ) - 1;
				}

				// Otherwise pop off stack and then return index
				u32 idx = mIndexFreeList.front( );

				// Swap / pop free list
				if ( mIndexFreeList.size( ) > 1 )
				{
					std::iter_swap( mIndexFreeList.begin( ), mIndexFreeList.end( ) - 1 );
					mIndexFreeList.pop_back( );
				}
				else
				{
					mIndexFreeList.clear( );
				}

				return idx;
			}

		private:
			mutable std::vector< T >	mData;							// Use the index from the handle vector to get index into this array of actual resources
			mutable std::vector< u32 >	mhandleIndices;					// Indices into this vector are returned to the user as handles
			mutable std::vector< u32 >	mReverseIndirectionIndices;		// Indices into this vector give reverse indirection into handle indices from objects
			mutable std::vector< u32 >	mIndexFreeList;					// Free list of most available indices
	}; 
}


#endif