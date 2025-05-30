const manifest = {
  name: "KnurToryTroller",
  short_name: "KNTTRL",
  icons: [
    {
      src: "http://localhost:5173/pig0_512.webp",
      sizes: "128x128 192x192 512x512",
      type: "image/webp",
    },
  ],
  description: "The only true Troller for KnurTory",
  start_url: "http://localhost:5173/",
  display: "standalone",
  theme_color: "#231F20",
  background_color: "#231F20",
};

export default JSON.stringify(manifest);
