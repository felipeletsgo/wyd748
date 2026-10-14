import type { APIRoute } from 'astro';
import { clientRelease } from '../data/public-content';

export const GET: APIRoute = () => new Response(JSON.stringify(clientRelease), {
  headers: { 'Content-Type': 'application/json; charset=utf-8' },
});
