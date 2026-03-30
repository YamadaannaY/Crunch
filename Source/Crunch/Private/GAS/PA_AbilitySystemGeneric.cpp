#include "GAS/PA_AbilitySystemGeneric.h"

const FRealCurve* UPA_AbilitySystemGeneric::GetExperienceCurve() const
{
	return ExperienceCurveTable->FindCurve(ExperienceRowName,"");
}
