// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemTreeWidget.h"

#include "SplineWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UItemTreeWidget::DrawFromNode(const ITreeNodeInterface* NodeInterface)
{
	if (!NodeInterface) return ;

	//如果当前中心Item就是形参的Node，说明已经进行过以此开始递归的Draw，不再进行一次
	if (CurrentCenterItem==NodeInterface->GetItemObject()) return ;

	//清空Canvas，复制CenterItem为形参的Node
	ClearTree();
	CurrentCenterItem=NodeInterface->GetItemObject();

	float NextLeafXPos=0.f;
	UCanvasPanelSlot* CenterWidgetPanelSlot=nullptr;
	//创建ItemWidget并返回Slot
	UUserWidget* CenterWidget=CreateWidgetForNode(NodeInterface,CenterWidgetPanelSlot);

	TArray<UCanvasPanelSlot*> LowerStreamSlots,UpperStreamSlots;

	//调用递归回调，将IngredientMap递归一次
	DrawStream(false,NodeInterface,CenterWidget,CenterWidgetPanelSlot,0,NextLeafXPos,LowerStreamSlots);

	float LowerStreamXMax=NextLeafXPos-NodeSize.X-NodeGap.X;
	float LowerMoveAmt=0.f-LowerStreamXMax/2.0f;
	for (UCanvasPanelSlot* StreamSlot: LowerStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D{LowerMoveAmt,0.f});
	}

	NextLeafXPos=0.f;
	//将UpperMap也递归一次
	DrawStream(true,NodeInterface,CenterWidget,CenterWidgetPanelSlot,0,NextLeafXPos,UpperStreamSlots);

	float UpperStreamXMax=NextLeafXPos-NodeSize.X-NodeGap.X;
	float UpperMoveAmt=0.f-UpperStreamXMax/2.0f;
	
	for (UCanvasPanelSlot* StreamSlot: UpperStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D{UpperMoveAmt,0.f});
	}
	//重置，留待下次使用
	CenterWidgetPanelSlot->SetPosition(FVector2D::ZeroVector);
}

void UItemTreeWidget::DrawStream(bool bUpperStream, const ITreeNodeInterface* StartingNodeInterface,
                                 UUserWidget* StartingNodeWidget,UCanvasPanelSlot* StartingNodeSlot,int StartingNodeDepth, float& NextLeafXPosition,
                                 TArray<UCanvasPanelSlot*>& OutStreamSlots)
{
	//确定是从哪一边开始Draw,对应获取Interface的CombinationMap和IngredientMap
	TArray<const ITreeNodeInterface*> NextTreeNodeInterfaces=bUpperStream ? StartingNodeInterface->GetInputs() : StartingNodeInterface->GetOutputs();

	//每个Node的垂直边加上Gap垂直边长度乘以深度即总垂直边深度
	float StartingNodeYPos=(NodeSize.Y+NodeGap.Y)* StartingNodeDepth * (bUpperStream ? -1 : 1);

	//如果没有，说明当前方向没有可联系的Item
	if (NextTreeNodeInterfaces.Num()==0)
	{
		//利用传入的XPos和根据深度计算得到的YPos确定Slot位置
		StartingNodeSlot->SetPosition(FVector2D{NextLeafXPosition,StartingNodeYPos});
		//引用修改XPos，增加下一个Slot的生成距离
		NextLeafXPosition+=NodeSize.X+NodeGap.X;
		return;
	}
	//可遍历，用一个Sum判断X方向的水平总长度
	float NextNodeXPosSum=0;
	
	for (const ITreeNodeInterface* NextTreeNodeInterface : NextTreeNodeInterfaces)
	{
		//创建新ItemSlot
		UCanvasPanelSlot* NextWidgetSlot;
		UUserWidget* NextWidget=CreateWidgetForNode(NextTreeNodeInterface,NextWidgetSlot);
		OutStreamSlots.Add(NextWidgetSlot);
		//建立新的Slot和当前Slot的联系
		if (bUpperStream)
		{
			CreateConnection(NextWidget,StartingNodeWidget);
		}
		else
		{
			CreateConnection(StartingNodeWidget,NextWidget);
		}
		//将新Slot的集合进行递归，深度+1，直到这一个Item的合成/原料表都已经递归完毕
		DrawStream(bUpperStream,NextTreeNodeInterface,NextWidget,NextWidgetSlot,StartingNodeDepth+1,NextLeafXPosition,OutStreamSlots);

		//记录X方向总长度
		NextNodeXPosSum+=NextWidgetSlot->GetPosition().X;
	}
	//修改起始Slot的X位置，水平设置在X方向的中心位置，竖直方向则抬高层数对应的Slot距离
	const float StartingNodeXPos=NextNodeXPosSum/NextTreeNodeInterfaces.Num();
	StartingNodeSlot->SetPosition(FVector2D{StartingNodeXPos,StartingNodeYPos});
}

void UItemTreeWidget::ClearTree()
{
	RootPanel->ClearChildren();
}

UUserWidget* UItemTreeWidget::CreateWidgetForNode(const ITreeNodeInterface* Node,UCanvasPanelSlot*& OutCanvasSlot)
{
	if (!Node) return nullptr;

	//获取接口对应的ItemWidget并加入到Canvas层级中，控件会自动拥有一个CanvasPanelSlot
	UUserWidget* NodeWidget=Node->GetWidget();
	OutCanvasSlot=RootPanel->AddChildToCanvas(NodeWidget);
	
	if (OutCanvasSlot)
	{
		//控制布局逻辑，分配到Canvas中心。设置Order比Spline高
		OutCanvasSlot->SetSize(NodeSize);
		OutCanvasSlot->SetAnchors(FAnchors(0.5f));
		OutCanvasSlot->SetAlignment(FVector2D(0.5f));
		OutCanvasSlot->SetZOrder(1);
	}
	return NodeWidget;
}

void UItemTreeWidget::CreateConnection(const UUserWidget* From, const UUserWidget* To)
{
	if (!From || !To) return;

	//创建Spline及其Slot
	USplineWidget* Connection=CreateWidget<USplineWidget>(GetOwningPlayer());
	UCanvasPanelSlot* ConnectionPanelSlot=RootPanel->AddChildToCanvas(Connection);

	//所有初始Spline的左上角与Canvas左上角重合,方便增减移动
	if (ConnectionPanelSlot)
	{
		ConnectionPanelSlot->SetAnchors(FAnchors(0.f));
		ConnectionPanelSlot->SetAlignment(FVector2D(0.f));
		ConnectionPanelSlot->SetPosition(FVector2D::Zero());
		ConnectionPanelSlot->SetZOrder(0);
	}

	//设置样条线位置，确定始末点方向和切线方向向量
	Connection->SetUpSpline(From,To,SourcePortLocalPos,DestinationPortLocalPos,SourcePortDirection,DestinationPortDirection);

	//设置样条线风格
	Connection->SetSplineStyle(ConnectionColor,ConnectionThickness);
}