import type { components } from '../lib/api.generated';

type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;
type NPC = components['schemas']['NPCDefinitionView'];
type Catalog = components['schemas']['NPCDefinitionCatalog'];
type Result = components['schemas']['NPCDefinitionResult'];

export function setupNPCEditor(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const text = (id: string, value: string) => { el(id).textContent = value; };
  const portuguese = document.documentElement.lang.toLowerCase().startsWith('pt') || navigator.language.toLowerCase().startsWith('pt');
  const copy = portuguese ? {
    title: 'Gerenciamento de NPCs', nav: 'NPCs', refresh: 'Atualizar', channel: 'Canal', loadChannel: 'Carregar canal', select: 'NPC ativo',
    waiting: 'Aguardando consulta.', help: 'Edite o JSON completo de um NPC ativo. O servidor valida e aplica a mudança somente neste canal.',
    json: 'JSON do NPC', reason: 'Motivo da alteração', reasonHint: 'Ex.: adicionar item solicitado', save: 'Salvar e aplicar', restore: 'Restaurar JSON de origem',
    unavailable: 'Este canal não está conectado a um servidor com PostgreSQL.', noNPCs: 'Nenhum NPC ativo editável foi encontrado neste canal.',
    selectNPC: 'Selecione um NPC.', source: 'Dados de origem', edited: 'Editado no PostgreSQL', lastSaved: 'Última alteração',
    saved: 'Definição salva e aplicada neste canal.', pending: 'Definição salva. Aguardando aplicação no canal; atualize para consultar.',
    applied: 'Aplicado', pendingState: 'Aplicação pendente', loadFirst: 'Carregue o canal selecionado antes de editar.',
    stale: 'Outro administrador alterou este NPC. A versão atual foi carregada; revise antes de salvar.',
    unknownNPC: 'Este NPC não está mais ativo neste canal.', notEditable: 'Este template não pode ser editado como NPC.',
    invalid: 'JSON inválido ou contém campos/valores não permitidos. Confira a estrutura e os itens.',
    permission: 'Esta sessão não tem permissão para editar NPCs.', unexpected: 'Resultado inesperado.',
    parse: 'O JSON contém erro de sintaxe.', discard: 'Descartar as alterações não salvas?',
    confirmSave: 'Salvar e aplicar esta definição no canal', confirmRestore: 'Restaurar a definição de origem no canal',
    invalidChannel: 'Informe um canal entre 1 e 255.', busy: 'O servidor está ocupado. Tente novamente.', saving: 'Salvando definição…',
    scope: 'Aplicar em', thisChannel: 'Canal selecionado', bothChannels: 'Ambos os canais (1 e 2)',
    helpBoth: 'Edite um NPC nos canais 1 e 2. O salvamento é conjunto; a aplicação é confirmada em cada canal.',
    confirmBothSave: 'Salvar e aplicar esta definição nos canais 1 e 2?', confirmBothRestore: 'Restaurar a definição de origem nos canais 1 e 2?',
    savedBoth: 'Definição salva e aplicada nos dois canais.', pendingBoth: 'Definição salva nos dois canais. Aguardando aplicação; confira as versões por canal.',
    peerMissing: 'Carregue os canais 1 e 2 com este NPC ativo antes de editar ambos.',
  } : {
    title: 'NPC management', nav: 'NPCs', refresh: 'Refresh', channel: 'Channel', loadChannel: 'Load channel', select: 'Active NPC',
    waiting: 'Waiting for a query.', help: 'Edit the complete JSON for an active NPC. The server validates and applies the change only to this channel.',
    json: 'NPC JSON', reason: 'Reason for the change', reasonHint: 'E.g. add requested item', save: 'Save and apply', restore: 'Restore source JSON',
    unavailable: 'This channel is not connected to a server with PostgreSQL.', noNPCs: 'No active editable NPCs were found in this channel.',
    selectNPC: 'Select an NPC.', source: 'Source definition', edited: 'Edited in PostgreSQL', lastSaved: 'Last changed',
    saved: 'Definition saved and applied to this channel.', pending: 'Definition saved. Waiting for channel application; refresh to check.',
    applied: 'Applied', pendingState: 'Application pending', loadFirst: 'Load the selected channel before editing.',
    stale: 'Another staff member changed this NPC. The current version was loaded; review it before saving.',
    unknownNPC: 'This NPC is no longer active in this channel.', notEditable: 'This template cannot be edited as an NPC.',
    invalid: 'Invalid JSON or disallowed fields/values. Check the structure and item references.',
    permission: 'This session cannot edit NPCs.', unexpected: 'Unexpected result.',
    parse: 'The JSON contains a syntax error.', discard: 'Discard unsaved changes?',
    confirmSave: 'Save and apply this definition to channel', confirmRestore: 'Restore the source definition in channel',
    invalidChannel: 'Enter a channel from 1 to 255.', busy: 'The server is busy. Try again.', saving: 'Saving definition…',
    scope: 'Apply to', thisChannel: 'Selected channel', bothChannels: 'Both channels (1 and 2)',
    helpBoth: 'Edit an NPC in channels 1 and 2. Saving is atomic; application is confirmed separately for each channel.',
    confirmBothSave: 'Save and apply this definition to channels 1 and 2?', confirmBothRestore: 'Restore the source definition in channels 1 and 2?',
    savedBoth: 'Definition saved and applied to both channels.', pendingBoth: 'Definition saved to both channels. Waiting for application; check each channel version.',
    peerMissing: 'Load channels 1 and 2 with this NPC active before editing both.',
  };
  let catalog: Catalog | undefined;
  let selected: NPC | undefined;
  let peerCatalog: Catalog | undefined;
  let loadedBoth = false;
  let originalJSON = '';
  let busy = false;
  let generation = 0;

  function explainNPCError(error: unknown) {
    const code = error && typeof error === 'object' && 'code' in error ? String((error as { code: unknown }).code) : '';
    if (code === 'npc_unavailable') return copy.unavailable;
    if (code === 'invalid_channel') return copy.invalidChannel;
    if (code === 'control_busy') return copy.busy;
    return explain(error);
  }

  const channelInput = el<HTMLInputElement>('npc-channel');
  const npcSelect = el<HTMLSelectElement>('npc-select');
  const editor = el<HTMLTextAreaElement>('npc-definition');
  const scopeInput = el<HTMLSelectElement>('npc-scope');

  function localize() {
    text('npc-management-title', copy.title);
    text('npc-editor-eyebrow', portuguese ? 'CONTEÚDO DO JOGO' : 'GAME CONTENT');
    text('npc-management-nav-label', copy.nav);
    el<HTMLButtonElement>('npc-editor-refresh').textContent = copy.refresh;
    text('npc-channel-label', copy.channel); el<HTMLButtonElement>('npc-channel-load').textContent = copy.loadChannel;
    text('npc-select-label', copy.select); text('npc-editor-help', copy.help); text('npc-definition-label', copy.json);
    text('npc-reason-label', copy.reason); el<HTMLInputElement>('npc-reason').placeholder = copy.reasonHint;
    el<HTMLButtonElement>('npc-save').textContent = copy.save; el<HTMLButtonElement>('npc-restore').textContent = copy.restore;
    text('npc-scope-label', copy.scope); text('npc-scope-channel', copy.thisChannel); text('npc-scope-both', copy.bothChannels);
  }

  function dirty() { return editor.value.trim() !== originalJSON; }
  function channelMatches() { return !!catalog && Number(channelInput.value) === catalog.channel && loadedBoth === (scopeInput.value === 'both'); }
  function peerNPC() { return peerCatalog?.npcs.find(npc => npc.name === selected?.name); }
  function canEdit() { return !!catalog?.available && !!selected?.active && channelMatches() && (!loadedBoth || (!!peerCatalog?.available && !!peerNPC()?.active)); }
  function targetState() {
    const views = selected ? [selected, ...(loadedBoth && peerNPC() ? [peerNPC()!] : [])] : [];
    text('npc-target-state', loadedBoth ? views.map(npc => `${copy.channel} ${npc.channel} · v${npc.version} · ${copy.applied} v${npc.appliedVersion} · ${npc.appliedVersion >= npc.version ? copy.applied : copy.pendingState}`).join(' | ') : '');
  }
  function updateButtons() {
    const hasNPC = canEdit();
    el<HTMLButtonElement>('npc-editor-refresh').disabled = busy;
    el<HTMLButtonElement>('npc-channel-load').disabled = busy;
    channelInput.disabled = busy;
    scopeInput.disabled = busy;
    npcSelect.disabled = busy || !catalog?.available || !channelMatches();
    editor.disabled = busy || !hasNPC;
    el<HTMLButtonElement>('npc-save').disabled = busy || !hasNPC;
    el<HTMLButtonElement>('npc-restore').disabled = busy || !hasNPC || !(selected?.overridden || (loadedBoth && peerNPC()?.overridden));
  }

  function applyNPC(npc?: NPC) {
    selected = npc;
    originalJSON = npc ? JSON.stringify(npc.definition, null, 2) : '';
    editor.value = originalJSON;
    const state = npc
      ? `${copy.channel} ${npc.channel} · ${npc.overridden ? copy.edited : copy.source} · v${npc.version} · ${npc.appliedVersion >= npc.version ? copy.applied : copy.pendingState}${npc.updatedAt ? ` · ${copy.lastSaved} ${npc.updatedBy || 'staff'} · ${new Date(npc.updatedAt).toLocaleString()}` : ''}`
      : copy.selectNPC;
    text('npc-editor-state', state);
    text('npc-editor-message', '');
    targetState();
    updateButtons();
  }

  async function refresh() {
    const channel = Number(channelInput.value);
    const both = scopeInput.value === 'both';
    if (!Number.isInteger(channel) || channel < 1 || channel > 255) {
      text('npc-editor-message', copy.invalidChannel);
      return;
    }
    if (both && channel !== 1 && channel !== 2) { text('npc-editor-message', copy.peerMissing); updateButtons(); return; }
    if (dirty() && !window.confirm(copy.discard)) { scopeInput.value = loadedBoth ? 'both' : 'channel'; updateButtons(); return; }
    const current = ++generation;
    catalog = undefined; peerCatalog = undefined; applyNPC(undefined); npcSelect.replaceChildren();
    busy = true; updateButtons();
    try {
      const [value, peer] = await Promise.all([
        api<Catalog>(`/staff/npcs?channel=${channel}`),
        both ? api<Catalog>(`/staff/npcs?channel=${channel === 1 ? 2 : 1}`) : Promise.resolve(undefined),
      ]);
      if (current !== generation) return;
      if (value.channel !== channel || (both && peer?.channel !== (channel === 1 ? 2 : 1))) throw new Error('Channel response mismatch');
      catalog = value; peerCatalog = peer; loadedBoth = both; selected = undefined;
      text('npc-editor-help', both ? copy.helpBoth : copy.help);
      npcSelect.replaceChildren(new Option(value.available ? copy.selectNPC : copy.unavailable, ''));
      for (const npc of value.npcs) npcSelect.add(new Option(`${npc.displayName} · ${npc.name}`, npc.name));
      applyNPC(undefined);
      if (!value.available) text('npc-editor-state', copy.unavailable);
      else if (!value.npcs.length) text('npc-editor-state', copy.noNPCs);
      text('npc-editor-message', '');
    } catch (error) {
      if (current === generation) text('npc-editor-message', explainNPCError(error));
    } finally {
      if (current === generation) { busy = false; updateButtons(); }
    }
  }

  async function save(restoreSource: boolean) {
    if (!selected || !catalog || busy) return;
    if (!channelMatches()) { text('npc-editor-message', copy.loadFirst); return; }
    if (!canEdit()) { text('npc-editor-message', loadedBoth ? copy.peerMissing : copy.loadFirst); return; }
    if (!window.confirm(loadedBoth ? (restoreSource ? copy.confirmBothRestore : copy.confirmBothSave) : `${restoreSource ? copy.confirmRestore : copy.confirmSave} ${catalog.channel}?`)) return;
    let definition: NPC['definition'] | undefined;
    if (!restoreSource) {
      try {
        const parsed: unknown = JSON.parse(editor.value);
        if (!parsed || typeof parsed !== 'object' || Array.isArray(parsed)) throw new Error('NPC JSON must be an object');
        definition = parsed as NPC['definition'];
      } catch {
        text('npc-editor-message', copy.parse);
        return;
      }
    }
    const current = ++generation;
    busy = true; updateButtons(); text('npc-editor-message', copy.saving);
    const command: components['schemas']['NPCDefinitionCommand'] = {
      version: 1, channel: catalog.channel, expectedVersion: selected.version, name: selected.name,
      reason: el<HTMLInputElement>('npc-reason').value.trim(), restoreSource, ...(definition ? { definition } : {}),
      ...(loadedBoth ? { targets: ([1, 2] as const).map(channel => ({ channel, expectedVersion: (selected!.channel === channel ? selected! : peerNPC()!).version })) } : {}),
    };
    try {
      const result = await api<Result>(`/staff/npcs/${encodeURIComponent(selected.name)}`, 'PUT', command);
      if (current !== generation) return;
      if (peerCatalog && result.npcs) {
        for (const npc of result.npcs) {
          if (npc.channel !== peerCatalog.channel || npc.name !== selected.name) continue;
          const index = peerCatalog.npcs.findIndex(entry => entry.name === npc.name);
          if (index >= 0) peerCatalog.npcs[index] = npc;
        }
      }
      if (result.code === 'ok' || result.code === 'saved_pending') {
        selected = result.npc;
        const index = catalog.npcs.findIndex(npc => npc.name === result.npc.name);
        if (index >= 0) catalog.npcs[index] = result.npc;
        applyNPC(result.npc);
        const confirmed = result.code === 'ok' && result.npc.appliedVersion >= result.npc.version && (!loadedBoth || [1, 2].every(channel => result.npcs?.some(npc => npc.channel === channel && npc.name === selected?.name && npc.appliedVersion >= npc.version)));
        text('npc-editor-message', loadedBoth ? (confirmed ? copy.savedBoth : copy.pendingBoth) : (confirmed ? copy.saved : copy.pending));
      } else if (result.code === 'stale_version') {
        selected = result.npc;
        const index = catalog.npcs.findIndex(npc => npc.name === result.npc.name);
        if (index >= 0) catalog.npcs[index] = result.npc;
        applyNPC(result.npc);
        text('npc-editor-message', copy.stale);
      } else {
        const messages: Record<string, string> = {
          unknown_npc: copy.unknownNPC, not_editable: copy.notEditable, invalid_definition: copy.invalid,
          invalid_command: copy.invalid, capability_required: copy.permission, unavailable: copy.unavailable,
        };
        text('npc-editor-message', messages[result.code] ?? copy.unexpected);
      }
    } catch (error) {
      if (current === generation) text('npc-editor-message', explainNPCError(error));
    } finally {
      if (current === generation) { busy = false; updateButtons(); }
    }
  }

  localize();
  channelInput.addEventListener('input', () => { updateButtons(); text('npc-editor-message', channelMatches() ? '' : copy.loadFirst); });
  scopeInput.addEventListener('change', () => { if (!busy) void refresh(); });
  editor.addEventListener('input', () => updateButtons());
  el<HTMLButtonElement>('npc-editor-refresh').addEventListener('click', () => void refresh());
  el<HTMLButtonElement>('npc-channel-load').addEventListener('click', () => void refresh());
  npcSelect.addEventListener('change', event => {
    const next = catalog?.npcs.find(npc => npc.name === (event.currentTarget as HTMLSelectElement).value);
    if (dirty() && !window.confirm(copy.discard)) {
      npcSelect.value = selected?.name ?? '';
      return;
    }
    applyNPC(next);
    if (loadedBoth && selected && !canEdit()) text('npc-editor-message', copy.peerMissing);
  });
  el<HTMLFormElement>('npc-definition-form').addEventListener('submit', event => { event.preventDefault(); void save(false); });
  el<HTMLButtonElement>('npc-restore').addEventListener('click', () => void save(true));
  return {
    refresh, reset() {
      generation++; catalog = undefined; peerCatalog = undefined; selected = undefined; originalJSON = ''; busy = false; loadedBoth = false; scopeInput.value = 'channel';
      npcSelect.replaceChildren(new Option(copy.selectNPC, '')); editor.value = '';
    text('npc-editor-help', copy.help); targetState();
      text('npc-editor-state', copy.waiting); text('npc-editor-message', ''); updateButtons();
    }
  };
}
