# Deploy to AWS with Terraform

First set up the AWS keys in your environment.

When you run these commands it will prompt you for a Github username. From this it downloads the public keys for the user and adds them as the ssh user for the EC2 instance.

```bash
terraform init
terraform apply
```

Once you are finished you can destroy everything with `terraform destroy`

## Example output

```
Outputs:

health_check_url = "http://44.202.90.183:5060/health-check"
ip_addresses_url = "http://44.202.90.183:8082/ip-addresses"
ssh_command = "ssh ubuntu@44.202.90.183"
```
