# Create an env file like:

# .sentrypeer_env
# SENTRYPEER_WEBHOOK_URL=https://sentrypeer.com/api/events
# SENTRYPEER_OAUTH2_CLIENT_ID=blah
# SENTRYPEER_OAUTH2_CLIENT_SECRET=blah
# SENTRYPEER_WEBHOOK=1

# Then run:
docker run -d -p 5060:5060 -p 8082:8082 --env-file .sentrypeer_env sentrypeer/sentrypeer:latest