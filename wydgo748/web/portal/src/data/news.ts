import { orderNews, parseNewsPost } from './news-format';

// Loaded at build time; a malformed post fails the build.
const files = import.meta.glob('../content/news/*.md', { query: '?raw', import: 'default', eager: true }) as Record<string, string>;

export const news = orderNews(Object.entries(files).map(([file, raw]) => parseNewsPost(file, raw)));
