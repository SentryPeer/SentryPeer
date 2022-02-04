provider "aws" {
  region = "us-east-1"
}

variable "github_user" {
  type = string
  description = "Use this users public keys for ssh"
}

resource "aws_instance" "sentypeer_instance" {
  ami           = "ami-04169656fea786776"
  instance_type = "t2.nano"
  user_data     = templatefile("install.sh", { github_user = var.github_user})
  tags = {
    Name  = "SentryPeer"
    Origin = "Terraform"
  }

  vpc_security_group_ids = [aws_security_group.web_sg.id]
}

resource "aws_security_group" "web_sg" {
  name = "sentrypeer-sg"
  ingress {
    from_port   = 8082
    to_port     = 8082
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    from_port   = 5060
    to_port     = 5060
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
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
  value       = "http://${aws_instance.sentypeer_instance.public_ip}:5060/health-check"
}

output "ip_addresses_url" {
  description = "URL for the ip addresses"
  value       = "http://${aws_instance.sentypeer_instance.public_ip}:8082/ip-addresses"
}

output "ssh_command" {
  description = "Log into the server with"
  value       = "ssh ubuntu@${aws_instance.sentypeer_instance.public_ip}"
}

