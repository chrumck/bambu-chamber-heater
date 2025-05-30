const manifest = {
  name: "KnurToryTroller",
  short_name: "KNTTRL",
  icons: [
    {
      src: "http://localhost:5173/pig0_512.png",
      sizes: "128x128 192x192 512x512",
      type: "image/png",
    },
  ],
  description: "The only true Troller for KnurTory",
  start_url: "http://localhost:5173/",
  display: "standalone",
  theme_color: "#231F20",
  background_color: "#231F20",
};

export default JSON.stringify(manifest);
