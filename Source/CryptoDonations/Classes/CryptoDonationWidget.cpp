#include "CryptoDonationWidget.h"
#include "Kismet/GameplayStatics.h"

void UCryptoDonationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GetGameInstance())
    {
        Subsystem = GetGameInstance()->GetSubsystem<UCryptoDonationsSubsystem>();

        if (Subsystem)
        {
            Subsystem->OnPaymentCreated.AddDynamic(this, &UCryptoDonationWidget::HandlePaymentCreated);
            Subsystem->OnPaymentConfirmed.AddDynamic(this, &UCryptoDonationWidget::HandlePaymentConfirmed);
            Subsystem->OnPaymentFailed.AddDynamic(this, &UCryptoDonationWidget::HandlePaymentFailed);
        }
    }
}

// ===============================
// START DONATION
// ===============================
void UCryptoDonationWidget::StartDonation(float Amount, FString Currency, FString UserId)
{
    if (Subsystem)
    {
        Subsystem->StartDonation(Amount, Currency, UserId);
    }
}

// ===============================
// EVENTS → BLUEPRINT
// ===============================
void UCryptoDonationWidget::HandlePaymentCreated(const FDonationSession& Session)
{
    OnPaymentCreatedBP(Session);
}

void UCryptoDonationWidget::HandlePaymentConfirmed(int32 Reward)
{
    OnPaymentConfirmedBP(Reward);
}

void UCryptoDonationWidget::HandlePaymentFailed(const FString& Error)
{
    OnPaymentFailedBP(Error);
}