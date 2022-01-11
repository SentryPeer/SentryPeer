<template>
  <div class="row">
    <div class="col-auto">
      <div class="card mb-5">
        <div class="card-header">Time Frame</div>
        <div class="card-body">
          <nav class="nav nav-pills flex-column">
            <a class="nav-link active" aria-current="page" href="#">1 Hour</a>
            <a class="nav-link" @click="ipAddressFilter(24)">24 Hours</a>
            <a class="nav-link" href="#">7 Days</a>
            <a class="nav-link" href="#">30 Days</a>
          </nav>
        </div>
      </div>
    </div>
    <div class="col">
      <div class="card mb-5">
        <div class="card-header">
          Total IP addresses collected between 17:00 and 18:00 UTC:
          <strong class="float-end">{{ ip_addresses_total }}</strong>
        </div>
        <div class="card-body">
          <table class="table">
            <thead>
              <tr>
                <th scope="col">IP Address</th>
                <th class="text-end" scope="col">Last seen</th>
                <th class="text-end" scope="col">Count</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="item in ipAddressFilter" :key="item">
                <td>{{ item.ip_address }}</td>
                <td class="text-end">
                  {{ $filters.timeAgo(item.seen_last) }}
                </td>
                <td class="text-end">{{ item.seen_count }}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>
<script>
import { useToast } from "vue-toastification"

export default {
  name: "SourceIPs",
  data() {
    return {
      name: "SourceIPs",
      ip_addresses: [],
      ip_address_total: null,
    }
  },
  methods: {
    getIPAddresses() {
      fetch("http://localhost:8082/ip-addresses")
        .then(async (response) => {
          const data = await response.json()

          // check for error response
          if (!response.ok) {
            // get error message from body or default to response statusText
            const error = (data && data.message) || response.statusText
            return Promise.reject(error)
          }

          this.ip_addresses = data.ip_addresses.filter((ip_address) =>
            this.$dayjs(ip_address.seen_last).isAfter(
              this.$dayjs().subtract(1, "hours")
            )
          )
          this.ip_addresses_total = data.ip_addresses_total
        })
        .catch((error) => {
          const toast = useToast()
          toast.error("SentryPeer API error: " + error.message)
        })
    },
    oneHour(ip_address) {
      return this.$dayjs(ip_address.seen_last).isAfter(
        this.$dayjs().subtract(1, "hours")
      )
    },
    twentyFourHours(ip_address) {
      return this.$dayjs(ip_address.seen_last).isAfter(
        this.$dayjs().subtract(24, "hours")
      )
    },
    sevenDays(ip_address) {
      return this.$dayjs(ip_address.seen_last).isAfter(
        this.$dayjs().subtract(7, "days")
      )
    },
    thirtyDays(ip_address) {
      return this.$dayjs(ip_address.seen_last).isAfter(
        this.$dayjs().subtract(30, "days")
      )
    },
  },
  created() {
    this.getIPAddresses()
  },
  computed: {
    ipAddressFilter(period) {
      if (period === 1) {
        return this.ip_addresses.filter((ip_address) =>
          this.oneHour(ip_address)
        )
      }
      if (period === 24) {
        return this.ip_addresses.filter((ip_address) =>
          this.twentyFourHours(ip_address)
        )
      }
      if (period === 7) {
        return this.ip_addresses.filter((ip_address) =>
          this.sevenDays(ip_address)
        )
      }
      if (period === 30) {
        return this.ip_addresses.filter((ip_address) =>
          this.thirtyDays(ip_address)
        )
      }
      return this.ip_addresses
    },
  },
}
</script>
