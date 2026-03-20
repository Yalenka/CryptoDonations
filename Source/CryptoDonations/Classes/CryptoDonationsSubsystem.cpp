#include "CryptoDonationsSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

void UCryptoDonationsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

// ===============================
// START DONATION
// ===============================
void UCryptoDonationsSubsystem::StartDonation(float Amount, FString Currency, FString UserId)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + "/create-payment");
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    FString Body = FString::Printf(
        TEXT("{\"amount\": %.2f, \"currency\": \"%s\", \"userId\": \"%s\"}"),
        Amount,
        *Currency,
        *UserId
    );

    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoDonationsSubsystem::CreatePaymentResponse);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("📡 Sending payment request..."));
}

// ===============================
// CREATE RESPONSE
// ===============================
void UCryptoDonationsSubsystem::CreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CreatePayment failed"));
        OnPaymentFailed.Broadcast("Network error");
        return;
    }

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("CreatePayment Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OnPaymentFailed.Broadcast("Invalid server response");
        return;
    }

    FDonationSession Session;

    if (JsonObject->HasField("payment_id"))
        Session.PaymentID = JsonObject->GetStringField("payment_id");

    if (JsonObject->HasField("pay_address"))
        Session.PayAddress = JsonObject->GetStringField("pay_address");

    if (JsonObject->HasField("pay_amount"))
        Session.PayAmount = JsonObject->GetStringField("pay_amount");

    if (JsonObject->HasField("qr_code_url"))
        Session.QRCodeURL = JsonObject->GetStringField("qr_code_url");

    CurrentPaymentID = Session.PaymentID;

    OnPaymentCreated.Broadcast(Session);

    // Start polling
    GetWorld()->GetTimerManager().SetTimer(
        PollTimer,
        this,
        &UCryptoDonationsSubsystem::CheckPaymentStatus,
        5.0f,
        true
    );
}

// ===============================
// POLLING
// ===============================
void UCryptoDonationsSubsystem::CheckPaymentStatus()
{
    if (CurrentPaymentID.IsEmpty()) return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + "/payment-status/" + CurrentPaymentID);
    Request->SetVerb("GET");

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoDonationsSubsystem::PaymentStatusResponse);
    Request->ProcessRequest();
}

// ===============================
// STATUS RESPONSE
// ===============================
void UCryptoDonationsSubsystem::PaymentStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid()) return;

    FString ResponseStr = Response->GetContentAsString();

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) return;

    FString Status = "unknown";
    int32 Reward = 0;

    if (JsonObject->HasField("status"))
        Status = JsonObject->GetStringField("status");

    if (JsonObject->HasField("reward"))
        Reward = JsonObject->GetIntegerField("reward");

    if (Status == "finished")
    {
        GetWorld()->GetTimerManager().ClearTimer(PollTimer);

        UE_LOG(LogTemp, Log, TEXT("Payment finished, reward: %d"), Reward);

        OnPaymentConfirmed.Broadcast(Reward);
    }
    else if (Status == "failed")
    {
        GetWorld()->GetTimerManager().ClearTimer(PollTimer);

        UE_LOG(LogTemp, Error, TEXT("Payment failed"));

        OnPaymentFailed.Broadcast("Payment failed");
    }
}
