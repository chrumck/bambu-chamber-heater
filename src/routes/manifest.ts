import { page } from "$app/state";

export const getManifestUrl = (iconUrl: string) =>
  URL.createObjectURL(
    new Blob(
      [
        JSON.stringify({
          name: "KnurToryTroller",
          short_name: "KNTTRL",
          icons: [
            {
              src: iconUrl,
              sizes: "128x128 192x192 512x512",
              type: "image/webp",
            },
          ],
          description: "The only true Troller for KnurTory",
          start_url: page.url.href,
          display: "standalone",
          theme_color: "#231F20",
          background_color: "#231F20",
        }),
      ],
      {
        type: "application/manifest+json",
      }
    )
  );
