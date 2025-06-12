/**
 * Converts the passed object to a Blob URL with the specified MIME type.
 */
export const getObjectUrl = (object: any, mimeType: string) =>
  URL.createObjectURL(new Blob([JSON.stringify(object)], { type: mimeType }));

/**
 * Fetches the the given data URL and converts it to a Blob URL.
 */
export const getObjectUrlFromUrl = (url: string) =>
  fetch(url).then((response) => response.blob().then(URL.createObjectURL));
