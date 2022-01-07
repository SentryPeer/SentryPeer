<template>
  <!-- Page content-->
  <div class="container">
    <div class="text-center my-5">
      <img
        class="mb-3"
        src="./assets/logo.svg"
        alt="SentryPeer Logo"
        style="height: 6rem"
      />
      <h1 class="mb-0">SIP Honeypot Data</h1>
    </div>
    <ul class="nav nav-pills nav-fill mb-5">
      <li class="nav-item">
        <router-link class="nav-link" :to="{ name: 'SourceIPs' }"
          >Source IPs</router-link
        >
      </li>
      <li class="nav-item">
        <router-link class="nav-link" :to="{ name: 'UserAgents' }"
          >User Agents Used
        </router-link>
      </li>
      <li class="nav-item">
        <router-link class="nav-link" :to="{ name: 'SipMethods' }"
          >Methods Used</router-link
        >
      </li>
      <li class="nav-item">
        <router-link class="nav-link" :to="{ name: 'CountryAnalysis' }"
          >Country Analysis</router-link
        >
      </li>
      <li class="nav-item">
        <router-link class="nav-link" :to="{ name: 'NumberAnalysis' }"
          >Number Analysis</router-link
        >
      </li>
    </ul>
    <router-view />
    <!-- The router will render the appropriate view -->
  </div>
</template>

<script>
export default {
  name: "App",
  metaInfo: {
    // if no subcomponents specify a metaInfo.title, this title will be used
    title: process.env.VUE_APP_DESCRIPTION,
    // all titles will be injected into this template
    titleTemplate: "%s - " + process.env.VUE_APP_DESCRIPTION,
  },
}
</script>
