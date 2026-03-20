#include "CryptoAPIClient.h"

    FString Body = FString::Printf(TEXT("{\"amount\": %.2f, \"currency\": \"%s\"}"), Amount, *Currency);
    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoAPIClient::OnCreatePaymentResponse);
    Request->ProcessRequest();
}

void UCryptoAPIClient::OnCreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid()) return;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        FDonationSession Session;
        Session.PaymentID = JsonObject->GetStringField("payment_id");
        Session.PayAddress = JsonObject->GetStringField("pay_address");
        Session.PayAmount = JsonObject->GetStringField("pay_amount");
        Session.QRCodeURL = JsonObject->GetStringField("qr_code_url");
        Session.Status = "waiting";

        OnPaymentCreated.Broadcast(Session);
    }
}

void UCryptoAPIClient::CheckPaymentStatus(FString PaymentID)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL("https://yourbackend.com/payment-status/" + PaymentID);
    Request->SetVerb("GET");

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoAPIClient::OnStatusResponse);
    Request->ProcessRequest();
}

void UCryptoAPIClient::OnStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid()) return;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        FString Status = JsonObject->GetStringField("status");
        OnPaymentStatus.Broadcast(Status);
    }
}