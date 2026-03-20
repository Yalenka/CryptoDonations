#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "TimerManager.h"
#include "CryptoDonationsSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDonationSession
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString PaymentID;

    UPROPERTY(BlueprintReadOnly)
    FString PayAddress;

    UPROPERTY(BlueprintReadOnly)
    FString PayAmount;

    UPROPERTY(BlueprintReadOnly)
    FString QRCodeURL;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentCreated, const FDonationSession&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentConfirmed, int32, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentFailed, const FString&, Error);

UCLASS()
class CRYPTODONATIONS_API UCryptoDonationsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable)
    void StartDonation(float Amount, FString Currency, FString UserId);

    UPROPERTY(BlueprintAssignable)
    FOnPaymentCreated OnPaymentCreated;

    UPROPERTY(BlueprintAssignable)
    FOnPaymentConfirmed OnPaymentConfirmed;

    UPROPERTY(BlueprintAssignable)
    FOnPaymentFailed OnPaymentFailed;

private:

    FString BackendURL = "http://127.0.0.1:3000";
    FString CurrentPaymentID;

    FTimerHandle PollTimer;

    void CreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
    void CheckPaymentStatus();
    void PaymentStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};
