#!/bin/bash
export PATH=/usr/local/bin:$PATH;

curl https://github.com/${github_user}.keys >> /home/ubuntu/.ssh/authorized_keys

apt-get update
apt-get upgrade -y
apt-get install -y apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"
apt-get update
apt-get install -y docker-ce
usermod -aG docker ubuntu

docker pull sentrypeer/sentrypeer
docker run -d -p 5060:5060 -p 8082:8082 sentrypeer/sentrypeer:latest