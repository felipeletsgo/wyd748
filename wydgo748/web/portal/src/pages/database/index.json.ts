import type { APIRoute } from 'astro';
import { catalogIndex } from '../../lib/catalog';
export const GET: APIRoute = () => new Response(JSON.stringify(catalogIndex), { headers: { 'Content-Type': 'application/json; charset=utf-8' } });
