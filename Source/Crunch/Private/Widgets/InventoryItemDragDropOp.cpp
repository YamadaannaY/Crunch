#include "InventoryItemDragDropOp.h"
#include "Widgets/InventoryItemWidget.h"
#include "Widgets/ItemWidget.h"

void UInventoryItemDragDropOp::SetDraggedItem(UInventoryItemWidget* DraggedItem)
{
	//PayLoad是一个通用数据容器，这里以目标UI作为对象
	Payload=DraggedItem;

	//创建一个ItemWidget类VisualWidget
	if (DragVisualClass)
	{
		//拖动时显示的Widget
		UItemWidget* DragItemWidget=CreateWidget<UItemWidget>(GetWorld(),DragVisualClass);
		if (DragItemWidget)
		{
			//为这个Widget设置以这个被拖动Item的Texture为Icon，并将其作为DragVisual
			DragItemWidget->SetIcon(DraggedItem->GetIconTexture());

			//UUserWidget类型的变量。当OnDragDetected时，创建的Widget赋值给这个属性后，它会自动跟随鼠标移动，直到松开按键。
			//即我们自定义的VisualClassWidget
			DefaultDragVisual=DragItemWidget;
		}
	}
}
