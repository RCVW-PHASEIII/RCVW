az containerapp up ^
  --name dab-container-app ^
  --subscription a6f557af-01f7-4f17-ba91-749e5eafc6f9 ^
  --resource-group rcvw-cbs ^
  --location eastus ^
  --environment dab-container-apps ^
  --image battelletmx1.azurecr.io/dab:latest ^
  --target-port 5000 ^
  --ingress external ^
  --query properties.configuration.ingress.fqdn
