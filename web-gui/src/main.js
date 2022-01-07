import { createApp } from "vue"
import App from "./App.vue"
import router from "./router"

// Local styles
import "../../web-gui-theme/dist/css/styles.css"

createApp(App).use(router).mount("#app")
