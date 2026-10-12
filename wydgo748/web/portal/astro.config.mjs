import { defineConfig } from 'astro/config';

export default defineConfig({
  output: 'static',
  trailingSlash: 'always',
  build: { inlineStylesheets: 'never' },
  vite: {
    build: { assetsInlineLimit: 0 },
    server: { proxy: {
      '/api/player': { target: 'http://127.0.0.1:8083', changeOrigin: false },
      '/api': { target: 'http://127.0.0.1:8082', changeOrigin: false },
    } },
  },
});
