provider "aws" {
  region = "us-east-1"
}

variable "github_user" {
  type        = string
  description = "Use this users public keys for ssh"
}

variable "management_ip" {
  type       = string
  description = "The IP address to allow SSH from"
}

resource "aws_instance" "sentypeer_instance" {
  ami           = "ami-0af9d24bd5539d7af" # Canonical, Ubuntu, 22.04 LTS, amd64 jammy image build on 2023-07-19
  instance_type = "t2.nano"
  user_data     = templatefile("install.sh", { github_user = var.github_user })
  tags          = {
    Name   = "SentryPeer"
    Origin = "Terraform"
  }

  vpc_security_group_ids = [aws_security_group.web_sg.id]
}

resource "aws_security_group" "web_sg" {
  name = "sentrypeer-sg"
  ingress {
    description = "Allow HTTP for API"
    from_port   = 8082
    to_port     = 8082
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "Allow SIP UDP"
    from_port   = 5060
    to_port     = 5060
    protocol    = "udp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "Allow SIP TCP"
    from_port   = 5060
    to_port     = 5060
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "Allow SSH from our management IP"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["${var.management_ip}/32"]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

output "health_check_url" {
  description = "URL for the health check"
  value       = "http://${aws_instance.sentypeer_instance.public_ip}:8082/health-check"
}

output "phone_numbers_url" {
  description = "URL for the phone numbers"
  value       = "http://${aws_instance.sentypeer_instance.public_ip}:8082/numbers"
}

output "ip_addresses_url" {
  description = "URL for the ip addresses"
  value       = "http://${aws_instance.sentypeer_instance.public_ip}:8082/ip-addresses"
}

output "ssh_command" {
  description = "Log into the server with"
  value       = "ssh ubuntu@${aws_instance.sentypeer_instance.public_ip}"
}

