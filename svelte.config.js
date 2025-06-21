import adapter from "@sveltejs/adapter-static";
import { vitePreprocess } from "@sveltejs/vite-plugin-svelte";

/** @type {import('@sveltejs/kit').Config} */
const config = {
  preprocess: vitePreprocess(),

  kit: {
    adapter: adapter({ fallback: "index.html" }),
    output: {
      bundleStrategy: "inline",
    },
    files: {
      assets: "src_front/static",
      lib: "src_front/lib",
      routes: "src_front/routes",
      appTemplate: "src_front/app.html",
    },
    typescript: {
      config: (config) => ({ ...config, include: [...config.include, "../src_front/**/*"] }),
    },
  },
};

export default config;
