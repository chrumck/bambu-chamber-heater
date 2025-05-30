const manifest = {
  name: "KnurToryTroller",
  short_name: "KNTTRL",
  icons: [
    {
      src: "./favicon.png",
      sizes: "128x128",
      type: "image/png",
    },
    {
      src: "./favicon.png",
      sizes: "192x192",
      type: "image/png",
    },
    {
      src: "./favicon.png",
      sizes: "512x512",
      type: "image/png",
    },
  ],
  description: "The only true Troller for KnurTory",
  start_url: "./",
  display: "standalone",
  theme_color: "#231F20",
  background_color: "#231F20",
};

export default JSON.stringify(manifest);
