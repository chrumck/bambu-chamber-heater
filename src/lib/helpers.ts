export const getObjectUrl = (object: any, mimeType: string) =>
  URL.createObjectURL(new Blob([JSON.stringify(object)], { type: mimeType }));
