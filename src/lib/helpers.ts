export const getObjectUrl = (object: any, mimeType: string) =>
  URL.createObjectURL(new Blob([JSON.stringify(object)], { type: mimeType }));

export const getObjectUrlFromUrl = (url: string) =>
  fetch(url).then((response) => response.blob().then(URL.createObjectURL));
