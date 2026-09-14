import { defineConfig } from 'astro/config';

export default defineConfig({
  output: 'static',
  trailingSlash: 'always',
  build: { inlineStylesheets: 'never' },
  vite: { server: { proxy: { '/api': { target: 'http://127.0.0.1:8082', changeOrigin: false } } } },
});
