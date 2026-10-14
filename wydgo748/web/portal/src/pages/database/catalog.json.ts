import type { APIRoute } from 'astro';
import { catalog } from '../../lib/catalog';
export const GET: APIRoute = () => new Response(JSON.stringify(catalog), { headers: { 'Content-Type': 'application/json; charset=utf-8' } });
