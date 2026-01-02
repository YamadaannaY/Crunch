// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineWidget.h"

void USplineWidget::SetUpSpline(const UUserWidget* InStartWidget, const UUserWidget* InEndWidget,
	const FVector2D& InStartPortLocalCoord, const FVector2D& InEndPortLocalCoord, const FVector2D& InStartPortDirection,
	const FVector2D& InEndPortDirection)
{
	//连线的起始点与结束点的Widget
	StartWidget=InStartWidget;
	EndWidget=InEndWidget;

	//定义起始点和结束点在Widget内部的相对位置
	StartPortLocalCoord=InStartPortLocalCoord;
	EndPortLocalCoord=InEndPortLocalCoord;

	//决定了曲线在起点和终点处的切线方向向量，用以扭曲样条线轨迹
	StartPortDirection=InStartPortDirection;
	EndPortDirection=InEndPortDirection;
}

void USplineWidget::SetSplineStyle(const FLinearColor& InColor, float InThickness)
{
	Color=InColor;
	Thickness=InThickness;
}

int32 USplineWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId=Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//测试的样条线Pos
	FVector2D StartPos=TestStartPos;
	FVector2D EndPos=TestEndPos;

	if (StartWidget && EndWidget)
	{
		//实际设置的Pos
		//GetCachedGeometry用与将线段的端点是动态绑定到两个Widget上。如果这两个Widget在屏幕上移动，线条会自动重绘并跟随。
		StartPos=StartWidget->GetCachedGeometry().GetLocalPositionAtCoordinates(StartPortLocalCoord);
		EndPos=EndWidget->GetCachedGeometry().GetLocalPositionAtCoordinates(EndPortLocalCoord);
	}

	//将逻辑坐标转换为屏幕上的视觉线条。
	//++LayerId确保Spline在父Widget上方
	FSlateDrawElement::MakeSpline(OutDrawElements,++LayerId,AllottedGeometry.ToPaintGeometry(),StartPos,StartPortDirection,
		EndPos,EndPortDirection,Thickness,ESlateDrawEffect::None,Color);

	return LayerId;
}
