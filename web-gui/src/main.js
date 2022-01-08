import { createApp } from "vue"
import App from "./App.vue"
import router from "./router"

// Plugins
import Toast from "vue-toastification"

// Local styles
import "../../web-gui-theme/dist/css/styles.css"
import "vue-toastification/dist/index.css"

const app = createApp(App)
app.use(router)

// Use plugins
app.use(Toast, {
  position: "top-right",
  duration: 3000,
  closeOnClick: true,
  pauseOnHover: true,
  draggable: true,
  draggablePercent: 80,
  progress: true,
  progressSteps: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
})

app.mount("#app")
