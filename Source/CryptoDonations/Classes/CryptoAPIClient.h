#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DonationSession.h"
#include "CryptoAPIClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentCreated, const FDonationSession&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentStatus, const FString&, Status);

UCLASS()
class CRYPTODONATIONS_API UCryptoAPIClient : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FOnPaymentCreated OnPaymentCreated;

    UPROPERTY(BlueprintAssignable)
    FOnPaymentStatus OnPaymentStatus;

    UFUNCTION(BlueprintCallable)
    void CreateDonation(float Amount, FString Currency);

    void CheckPaymentStatus(FString PaymentID);

private:
    void OnCreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
    void OnStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};