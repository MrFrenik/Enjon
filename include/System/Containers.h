#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "System/Types.h"

#include <queue>
#include <vector>
#include <unordered_set>
#include <utility>
#include <functional>
#include <iostream>

namespace Enjon {

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


}


#endif