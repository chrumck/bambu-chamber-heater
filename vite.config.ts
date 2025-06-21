import { sveltekit } from "@sveltejs/kit/vite";
import { defineConfig } from "vite";

export default defineConfig({
  plugins: [sveltekit()],
  build: {
    assetsInlineLimit: Infinity,
  },
  server: {
    port: 5173,
    strictPort: true,
    host: "0.0.0.0",
  },
});
