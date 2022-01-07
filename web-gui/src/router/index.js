import { createRouter, createWebHistory } from "vue-router"
import SourceIPs from "../views/SourceIPs.vue"
import UserAgents from "../views/UserAgents.vue"
import SipMethods from "../views/SipMethods.vue"
import CountryAnalysis from "../views/CountryAnalysis.vue"
import NumberAnalysis from "../views/NumberAnalysis.vue"

const routes = [
  {
    path: "/",
    name: "SourceIPs",
    component: SourceIPs,
  },
  {
    path: "/user-agents",
    name: "UserAgents",
    component: UserAgents,
    // route level code-splitting
    // this generates a separate chunk (about.[hash].js) for this route
    // which is lazy-loaded when the route is visited.
    //component: () =>
    //  import(/* webpackChunkName: "about" */ "../views/About.vue"),
  },
  {
    path: "/sip-methods",
    name: "SipMethods",
    component: SipMethods,
  },
  {
    path: "/country-analysis",
    name: "CountryAnalysis",
    component: CountryAnalysis,
  },
  {
    path: "/number-analysis",
    name: "NumberAnalysis",
    component: NumberAnalysis,
  },
]

const router = createRouter({
  linkActiveClass: "active",
  history: createWebHistory(process.env.BASE_URL),
  routes,
})

export default router
