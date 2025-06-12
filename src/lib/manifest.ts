import { page } from "$app/state";

export const getManifest = (iconSrc: string) => ({
  name: "KnurToryTroller",
  short_name: "KNTTRL",
  icons: [
    {
      src: iconSrc,
      sizes: "128x128 192x192 512x512",
      type: "image/webp",
    },
  ],
  description: "The only true Troller for KnurTory",
  start_url: page.url.href,
  display: "standalone",
  theme_color: "#262626",
  background_color: "#262626",
});
